#include "game_logic.H"

GameLogic::GameLogic(const std::vector<Lubang>& pockets, Koordinat cueStartPos)
    : pockets_(pockets), cueStartPos_(cueStartPos) {}

void GameLogic::onShotTaken() {
    if (state_ != GameState::Aiming) return;
    state_ = GameState::BallsMoving;
    pocketedThisTurn_ = 0;
    cuePocketedThisTurn_ = false;
    lastFoul_ = false;
}

void GameLogic::update(std::vector<Bola>& balls, float dt) {
    if (state_ != GameState::BallsMoving) return;

    for (size_t i = 0; i < balls.size(); ++i) {
        balls[i].update(dt);
    }

    for (size_t i = 0; i < balls.size(); ++i) {
        for (size_t j = i + 1; j < balls.size(); ++j) {
            balls[i].cekTabrakanBola(balls[j]);
        }
    }

    checkPockets(balls);

    if (allBallsStopped(balls)) {
        endTurn(balls);
    }
}

void GameLogic::checkPockets(std::vector<Bola>& balls) {
    for (size_t i = 0; i < balls.size(); ++i) {
        if (!balls[i].isAktif()) continue;

        bool masuk = false;
        for (size_t j = 0; j < pockets_.size(); ++j) {
            if (pockets_[j].Apakah_bola_masuk(balls[i])) {
                masuk = true;
                break;
            }
        }

        if (!masuk) continue;

        balls[i].setStatus(false);
        balls[i].setKecepatan({0.f, 0.f});

        if (balls[i].getNomor() == 0) {
            cuePocketedThisTurn_ = true;
        } else {
            ++pocketedThisTurn_;
            scores_[currentPlayer_ - 1] += 1;
        }
    }
}

bool GameLogic::allBallsStopped(const std::vector<Bola>& balls) {
    for (size_t i = 0; i < balls.size(); ++i) {
        if (!balls[i].isAktif()) continue;
        if (balls[i].sedangBergerak()) return false;
    }
    return true;
}

void GameLogic::endTurn(std::vector<Bola>& balls) {
    if (cuePocketedThisTurn_) {
        lastFoul_ = true;
        for (size_t i = 0; i < balls.size(); ++i) {
            if (balls[i].getNomor() == 0) {
                balls[i].setStatus(true);
                balls[i].setposisi(cueStartPos_);
                balls[i].setKecepatan({0.f, 0.f});
                balls[i].update(0.f);
            }
        }
        switchPlayer();
    } else if (pocketedThisTurn_ == 0) {
        switchPlayer();
    }

    bool anyLeft = false;
    for (size_t i = 0; i < balls.size(); ++i) {
        if (balls[i].getNomor() != 0 && balls[i].isAktif()) {
            anyLeft = true;
            break;
        }
    }

    if (!anyLeft) {
        state_ = GameState::GameOver;
        if (scores_[0] > scores_[1])      winner_ = 1;
        else if (scores_[1] > scores_[0]) winner_ = 2;
        else                              winner_ = 0;
        return;
    }

    state_ = GameState::Aiming;
}

void GameLogic::switchPlayer() {
    currentPlayer_ = (currentPlayer_ == 1) ? 2 : 1;
}