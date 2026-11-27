#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "Bola.hpp"
#include "Lubang.hpp"
#include "Koordinat.hpp"

enum class GameState {
    Aiming,
    BallsMoving,
    GameOver
};

// Grup bola pemain pada 8-Ball. "Belum" = meja masih terbuka (grup belum ditentukan).
enum class GrupBola {
    Belum,
    Polos,    // bola 1-7
    Stripes   // bola 9-15
};

class GameLogic {
public:
    // footSpot = titik apex segitiga; dipakai untuk menaruh ulang bola 8 jika masuk saat break.
    GameLogic(const std::vector<Lubang>& pockets, Koordinat cueStartPos, Koordinat footSpot);

    // Panggil tepat setelah bola putih dipukul (butuh daftar bola untuk mencatat kondisi awal tembakan).
    void onShotTaken(const std::vector<Bola>& balls);
    void update(std::vector<Bola>& balls, float dt);

    GameState getState() const { return state_; }
    int  getCurrentPlayer() const { return currentPlayer_; }
    // Skor = jumlah bola grup pemain yang sudah masuk lubang (0 selama meja masih terbuka).
    int  getScore(int player) const;
    int  getWinner() const { return winner_; }
    bool wasFoul() const { return lastFoul_; }
    const std::string& getFoulReason() const { return foulReason_; }
    const std::string& getGameOverReason() const { return gameOverReason_; }

    GrupBola getGrupPemain(int player) const { return grup_[player - 1]; }
    bool isTableOpen() const { return grup_[0] == GrupBola::Belum; }
    bool isBreakShot() const { return breakPending_; }

    static GrupBola grupDariNomor(int nomor);
    static bool allBallsStopped(const std::vector<Bola>& balls);

private:
    void checkPockets(std::vector<Bola>& balls);
    void endTurn(std::vector<Bola>& balls);
    void switchPlayer();
    bool firstContactLegal(int nomor) const;
    void respotBola8(std::vector<Bola>& balls);

    std::vector<Lubang> pockets_;
    Koordinat cueStartPos_;
    Koordinat footSpot_;

    GameState state_ = GameState::Aiming;
    int currentPlayer_ = 1;
    int winner_ = 0;
    std::string gameOverReason_;

    // Status aturan 8-Ball
    GrupBola grup_[2] = {GrupBola::Belum, GrupBola::Belum};
    int  pocketedByGroup_[2] = {0, 0};   // [0] polos, [1] stripes
    bool breakPending_ = true;           // true sampai tembakan pertama selesai

    // Catatan tembakan yang sedang berjalan
    std::vector<int> pocketedOrder_;     // nomor bola (bukan putih) sesuai urutan masuk
    bool cuePocketedThisTurn_ = false;
    int  firstContact_ = -1;             // nomor bola pertama yang disentuh bola putih
    bool railAfterContact_ = false;
    int  railBallsMask_ = 0;             // bit n = bola n menyentuh rel (untuk aturan break)
    bool shooterOnEight_ = false;        // grup penembak sudah habis saat tembakan dimulai

    bool lastFoul_ = false;
    std::string foulReason_;
};