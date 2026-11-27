#include "game_logic.H"

GameLogic::GameLogic(const std::vector<Lubang>& pockets, Koordinat cueStartPos, Koordinat footSpot)
    : pockets_(pockets), cueStartPos_(cueStartPos), footSpot_(footSpot) {}

GrupBola GameLogic::grupDariNomor(int nomor) {
    if (nomor >= 1 && nomor <= 7)  return GrupBola::Polos;
    if (nomor >= 9 && nomor <= 15) return GrupBola::Stripes;
    return GrupBola::Belum;
}

int GameLogic::getScore(int player) const {
    GrupBola g = grup_[player - 1];
    if (g == GrupBola::Belum) return 0;
    return pocketedByGroup_[g == GrupBola::Polos ? 0 : 1];
}

void GameLogic::onShotTaken(const std::vector<Bola>& balls) {
    if (state_ != GameState::Aiming) return;
    state_ = GameState::BallsMoving;

    pocketedOrder_.clear();
    cuePocketedThisTurn_ = false;
    firstContact_ = -1;
    railAfterContact_ = false;
    railBallsMask_ = 0;
    lastFoul_ = false;
    foulReason_.clear();

    // Catat apakah penembak sudah menghabiskan grupnya (berarti sasarannya bola 8).
    shooterOnEight_ = false;
    GrupBola g = grup_[currentPlayer_ - 1];
    if (g != GrupBola::Belum) {
        int sisa = 0;
        for (size_t i = 0; i < balls.size(); ++i) {
            if (balls[i].isAktif() && grupDariNomor(balls[i].getNomor()) == g) ++sisa;
        }
        shooterOnEight_ = (sisa == 0);
    }
}

void GameLogic::update(std::vector<Bola>& balls, float dt) {
    if (state_ != GameState::BallsMoving) return;

    for (size_t i = 0; i < balls.size(); ++i) {
        balls[i].update(dt);
    }

    for (size_t i = 0; i < balls.size(); ++i) {
        for (size_t j = i + 1; j < balls.size(); ++j) {
            if (balls[i].cekTabrakanBola(balls[j])) {
                // Sentuhan pertama bola putih = bola sasaran yang pertama kali ditabrak
                if (firstContact_ == -1) {
                    if (balls[i].getNomor() == 0)      firstContact_ = balls[j].getNomor();
                    else if (balls[j].getNomor() == 0) firstContact_ = balls[i].getNomor();
                }
            }
        }
    }

    // Rel: hanya dihitung setelah bola putih menyentuh bola sasaran
    for (size_t i = 0; i < balls.size(); ++i) {
        if (!balls[i].ambilDanResetRel()) continue;
        int n = balls[i].getNomor();
        if (n != 0) railBallsMask_ |= (1 << n);
        if (firstContact_ != -1) railAfterContact_ = true;
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

        int n = balls[i].getNomor();
        if (n == 0) {
            cuePocketedThisTurn_ = true;
        } else {
            pocketedOrder_.push_back(n);
            GrupBola g = grupDariNomor(n);
            if (g != GrupBola::Belum) ++pocketedByGroup_[g == GrupBola::Polos ? 0 : 1];
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

bool GameLogic::firstContactLegal(int nomor) const {
    if (isTableOpen()) return nomor != 8;     // meja terbuka: boleh bola mana pun kecuali 8
    if (shooterOnEight_) return nomor == 8;   // grup habis: wajib bola 8
    return nomor != 8 && grupDariNomor(nomor) == grup_[currentPlayer_ - 1];
}

void GameLogic::respotBola8(std::vector<Bola>& balls) {
    for (size_t i = 0; i < balls.size(); ++i) {
        if (balls[i].getNomor() != 8) continue;

        // Geser dari foot spot menjauhi bola putih (ke arah rel foot) sampai tidak menimpa bola lain
        const float arahGeser = (footSpot_.x < cueStartPos_.x) ? -1.f : 1.f;
        Koordinat pos = footSpot_;
        for (int coba = 0; coba < 12; ++coba) {
            bool bentrok = false;
            for (size_t k = 0; k < balls.size(); ++k) {
                if (k == i || !balls[k].isAktif()) continue;
                float dx = balls[k].getposisi().x - pos.x;
                float dy = balls[k].getposisi().y - pos.y;
                if (dx * dx + dy * dy < 41.f * 41.f) { bentrok = true; break; }
            }
            if (!bentrok) break;
            pos.x += arahGeser * 41.f;
        }
        balls[i].setStatus(true);
        balls[i].setposisi(pos);
        balls[i].setKecepatan({0.f, 0.f});
        balls[i].update(0.f);   // sinkronkan posisi visual
    }
}

void GameLogic::endTurn(std::vector<Bola>& balls) {
    const int penembak = currentPlayer_;
    const int lawan = (penembak == 1) ? 2 : 1;
    const bool tembakanBreak = breakPending_;
    breakPending_ = false;

    bool bola8Masuk = false;
    bool bolaLainMasuk = false;   // selain bola putih dan bola 8
    for (size_t i = 0; i < pocketedOrder_.size(); ++i) {
        if (pocketedOrder_[i] == 8) bola8Masuk = true;
        else bolaLainMasuk = true;
    }

    // ---- Deteksi foul ----
    bool foul = false;
    std::string alasan;
    auto tandaiFoul = [&](const char* a) { if (!foul) { foul = true; alasan = a; } };

    if (cuePocketedThisTurn_) tandaiFoul("Bola putih masuk lubang");

    if (firstContact_ == -1) {
        tandaiFoul("Bola putih tidak menyentuh bola apa pun");
    } else {
        if (!firstContactLegal(firstContact_)) {
            if (shooterOnEight_)         tandaiFoul("Harus menyentuh bola 8 lebih dulu");
            else if (firstContact_ == 8) tandaiFoul("Menyentuh bola 8 lebih dulu");
            else                         tandaiFoul("Menyentuh bola lawan lebih dulu");
        }

        if (tembakanBreak) {
            int jumlahRel = 0;
            for (int n = 1; n <= 15; ++n) if (railBallsMask_ & (1 << n)) ++jumlahRel;
            if (pocketedOrder_.empty() && jumlahRel < 4)
                tandaiFoul("Break tidak sah (kurang dari 4 bola menyentuh rel)");
        } else if (pocketedOrder_.empty() && !railAfterContact_) {
            tandaiFoul("Tidak ada bola yang menyentuh rel setelah tumbukan");
        }
    }

    // Bola putih yang masuk dikembalikan ke titik awal (Langkah 3 akan menggantinya dengan ball-in-hand)
    if (cuePocketedThisTurn_) {
        for (size_t i = 0; i < balls.size(); ++i) {
            if (balls[i].getNomor() == 0) {
                balls[i].setStatus(true);
                balls[i].setposisi(cueStartPos_);
                balls[i].setKecepatan({0.f, 0.f});
                balls[i].update(0.f);
            }
        }
    }

    // ---- Bola 8 masuk ----
    if (bola8Masuk) {
        if (tembakanBreak) {
            respotBola8(balls);          // masuk saat break: bola 8 ditaruh kembali, permainan lanjut
            bola8Masuk = false;
        } else {
            bool kalah = foul || !shooterOnEight_;
            state_ = GameState::GameOver;
            winner_ = kalah ? lawan : penembak;
            if (!kalah)       gameOverReason_ = "Memasukkan bola 8 secara sah";
            else if (foul)    gameOverReason_ = "Bola 8 masuk saat foul (" + alasan + ")";
            else              gameOverReason_ = "Bola 8 masuk sebelum semua bola grupnya habis";
            lastFoul_ = foul;
            foulReason_ = alasan;
            return;
        }
    }

    // ---- Penentuan grup (meja terbuka, bukan break, tidak foul) ----
    if (!foul && isTableOpen() && !tembakanBreak) {
        for (size_t i = 0; i < pocketedOrder_.size(); ++i) {
            GrupBola g = grupDariNomor(pocketedOrder_[i]);
            if (g == GrupBola::Belum) continue;
            grup_[penembak - 1] = g;
            grup_[lawan - 1] = (g == GrupBola::Polos) ? GrupBola::Stripes : GrupBola::Polos;
            break;
        }
    }

    // ---- Giliran berikutnya ----
    bool lanjut = false;
    if (!foul) {
        if (tembakanBreak) {
            lanjut = bolaLainMasuk;
        } else {
            for (size_t i = 0; i < pocketedOrder_.size(); ++i) {
                int n = pocketedOrder_[i];
                if (n != 8 && grupDariNomor(n) == grup_[penembak - 1]) { lanjut = true; break; }
            }
        }
    }

    lastFoul_ = foul;
    foulReason_ = alasan;
    if (foul || !lanjut) switchPlayer();

    state_ = GameState::Aiming;
}

void GameLogic::switchPlayer() {
    currentPlayer_ = (currentPlayer_ == 1) ? 2 : 1;
}