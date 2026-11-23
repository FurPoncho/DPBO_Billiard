#include "game_logic.H"
#include <cmath>

GameLogic::GameLogic(const sf::FloatRect& playArea,
                     const std::vector<Pocket>& pockets,
                     sf::Vector2f cueStartPos)
    : playArea_(playArea), pockets_(pockets), cueStartPos_(cueStartPos) {}

// ------------------------------------------------------------
// Dipanggil saat pemain memukul bola putih
// ------------------------------------------------------------
void GameLogic::onShotTaken() {
    if (state_ != GameState::Aiming) return;
    state_ = GameState::BallsMoving;
    pocketedThisTurn_ = 0;
    cuePocketedThisTurn_ = false;
    lastFoul_ = false;
}

// ------------------------------------------------------------
// Update tiap frame
// ------------------------------------------------------------
void GameLogic::update(std::vector<Ball>& balls, float dt) {
    if (state_ != GameState::BallsMoving) return;

    // Gerakkan bola (hapus bagian ini kalau Mufti sudah menggerakkan bola di fisik_game.cpp)
    for (auto& b : balls) {
        if (!b.pocketed) b.pos += b.vel * dt;
    }

    bounceOffWalls(balls);
    resolveBallCollisions(balls);
    applyFriction(balls, dt);
    checkPockets(balls);

    if (allBallsStopped(balls)) {
        endTurn(balls);
    }
}

// ------------------------------------------------------------
// Gesekan meja: kecepatan dikurangi tiap frame
// ------------------------------------------------------------
void GameLogic::applyFriction(std::vector<Ball>& balls, float dt) {
    // Normalisasi ke 60 FPS agar konsisten walau frame rate berubah
    float factor = std::pow(FRICTION, dt * 60.f);
    for (auto& b : balls) {
        if (b.pocketed) continue;
        b.vel *= factor;
        float speed = std::hypot(b.vel.x, b.vel.y);
        if (speed < STOP_SPEED) b.vel = {0.f, 0.f};
    }
}

// ------------------------------------------------------------
// Pantulan dinding: balik komponen kecepatan yang sesuai
// ------------------------------------------------------------
void GameLogic::bounceOffWalls(std::vector<Ball>& balls) {
    float left   = playArea_.position.x;
    float right  = playArea_.position.x + playArea_.size.x;
    float top    = playArea_.position.y;
    float bottom = playArea_.position.y + playArea_.size.y;

    for (auto& b : balls) {
        if (b.pocketed) continue;

        if (b.pos.x - b.radius < left) {
            b.pos.x = left + b.radius;
            b.vel.x = -b.vel.x * WALL_DAMPING;
        } else if (b.pos.x + b.radius > right) {
            b.pos.x = right - b.radius;
            b.vel.x = -b.vel.x * WALL_DAMPING;
        }

        if (b.pos.y - b.radius < top) {
            b.pos.y = top + b.radius;
            b.vel.y = -b.vel.y * WALL_DAMPING;
        } else if (b.pos.y + b.radius > bottom) {
            b.pos.y = bottom - b.radius;
            b.vel.y = -b.vel.y * WALL_DAMPING;
        }
    }
}

// ------------------------------------------------------------
// Tumbukan antar bola (massa sama, tumbukan lenting)
// ------------------------------------------------------------
void GameLogic::resolveBallCollisions(std::vector<Ball>& balls) {
    for (size_t i = 0; i < balls.size(); ++i) {
        if (balls[i].pocketed) continue;
        for (size_t j = i + 1; j < balls.size(); ++j) {
            if (balls[j].pocketed) continue;

            Ball& a = balls[i];
            Ball& b = balls[j];

            sf::Vector2f diff = b.pos - a.pos;
            float dist = std::hypot(diff.x, diff.y);
            float minDist = a.radius + b.radius;

            if (dist >= minDist || dist == 0.f) continue;

            // Normal tumbukan
            sf::Vector2f n = diff / dist;

            // Pisahkan bola yang tumpang tindih
            float overlap = minDist - dist;
            a.pos -= n * (overlap / 2.f);
            b.pos += n * (overlap / 2.f);

            // Kecepatan relatif sepanjang normal
            sf::Vector2f relVel = a.vel - b.vel;
            float vn = relVel.x * n.x + relVel.y * n.y;
            if (vn <= 0.f) continue; // sudah saling menjauh

            // Massa sama: tukar komponen kecepatan sepanjang normal
            a.vel -= n * vn;
            b.vel += n * vn;
        }
    }
}

// ------------------------------------------------------------
// Bola masuk lubang
// ------------------------------------------------------------
bool GameLogic::isInPocket(const Ball& b) const {
    for (const auto& p : pockets_) {
        float dx = b.pos.x - p.pos.x;
        float dy = b.pos.y - p.pos.y;
        if (std::hypot(dx, dy) <= p.radius) return true;
    }
    return false;
}

void GameLogic::checkPockets(std::vector<Ball>& balls) {
    for (auto& b : balls) {
        if (b.pocketed) continue;
        if (!isInPocket(b)) continue;

        b.pocketed = true;
        b.vel = {0.f, 0.f};

        if (b.number == 0) {
            cuePocketedThisTurn_ = true;   // foul
        } else {
            ++pocketedThisTurn_;
            scores_[currentPlayer_ - 1] += 1;
        }
    }
}

// ------------------------------------------------------------
// Semua bola berhenti?
// ------------------------------------------------------------
bool GameLogic::allBallsStopped(const std::vector<Ball>& balls) {
    for (const auto& b : balls) {
        if (b.pocketed) continue;
        if (b.vel.x != 0.f || b.vel.y != 0.f) return false;
    }
    return true;
}

// ------------------------------------------------------------
// Akhir giliran: cek foul, ganti pemain, cek menang
// ------------------------------------------------------------
void GameLogic::endTurn(std::vector<Ball>& balls) {
    // Foul: bola putih masuk lubang -> taruh ulang, giliran pindah
    if (cuePocketedThisTurn_) {
        lastFoul_ = true;
        for (auto& b : balls) {
            if (b.number == 0) {
                b.pocketed = false;
                b.pos = cueStartPos_;
                b.vel = {0.f, 0.f};
            }
        }
        switchPlayer();
    }
    // Tidak ada bola masuk -> giliran pindah
    else if (pocketedThisTurn_ == 0) {
        switchPlayer();
    }
    // Ada bola masuk -> pemain yang sama lanjut

    // Cek apakah semua bola target (selain putih) sudah masuk
    bool anyLeft = false;
    for (const auto& b : balls) {
        if (b.number != 0 && !b.pocketed) { anyLeft = true; break; }
    }
    if (!anyLeft) {
        state_ = GameState::GameOver;
        if (scores_[0] > scores_[1])      winner_ = 1;
        else if (scores_[1] > scores_[0]) winner_ = 2;
        else                              winner_ = 0; // seri
        return;
    }

    state_ = GameState::Aiming;
}

void GameLogic::switchPlayer() {
    currentPlayer_ = (currentPlayer_ == 1) ? 2 : 1;
}