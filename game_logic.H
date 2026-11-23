#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp> 
#include <vector>

// ============================================================
// CATATAN: struct Ball di bawah ini hanya contoh minimal.
// Kalau Mufti sudah punya struct/class Ball di fisik_game.h,
// pakai punya dia dan hapus definisi ini (cocokkan nama field).
// ============================================================
struct Ball {
    sf::Vector2f pos;
    sf::Vector2f vel;
    float radius = 10.f;
    int number = 0;          // 0 = bola putih (cue ball)
    bool pocketed = false;   // true jika sudah masuk lubang
};

struct Pocket {
    sf::Vector2f pos;
    float radius = 18.f;
};

enum class GameState {
    Aiming,       // pemain membidik / menunggu pukulan
    BallsMoving,  // bola masih bergerak setelah dipukul
    GameOver
};

class GameLogic {
public:
    GameLogic(const sf::FloatRect& playArea,
              const std::vector<Pocket>& pockets,
              sf::Vector2f cueStartPos);

    // Dipanggil sekali saat pemain memukul bola putih
    void onShotTaken();

    // Dipanggil tiap frame dari main loop (dt dalam detik)
    void update(std::vector<Ball>& balls, float dt);

    // --- Getter untuk UI (milik Pasha) ---
    GameState getState() const { return state_; }
    int  getCurrentPlayer() const { return currentPlayer_; }   // 1 atau 2
    int  getScore(int player) const { return scores_[player - 1]; }
    int  getWinner() const { return winner_; }                 // 0 = belum ada
    bool wasFoul() const { return lastFoul_; }

    // --- Fungsi bantu (public supaya mudah dites) ---
    static bool allBallsStopped(const std::vector<Ball>& balls);
    bool isInPocket(const Ball& b) const;

private:
    void applyFriction(std::vector<Ball>& balls, float dt);
    void bounceOffWalls(std::vector<Ball>& balls);
    void resolveBallCollisions(std::vector<Ball>& balls);
    void checkPockets(std::vector<Ball>& balls);
    void endTurn(std::vector<Ball>& balls);
    void switchPlayer();

    sf::FloatRect playArea_;
    std::vector<Pocket> pockets_;
    sf::Vector2f cueStartPos_;

    GameState state_ = GameState::Aiming;
    int currentPlayer_ = 1;
    int scores_[2] = {0, 0};
    int winner_ = 0;

    // Status giliran yang sedang berjalan
    int  pocketedThisTurn_ = 0;
    bool cuePocketedThisTurn_ = false;
    bool lastFoul_ = false;

    // Konstanta fisika sederhana (boleh disesuaikan / samakan dengan Mufti)
    static constexpr float FRICTION     = 0.985f; // pengali kecepatan per frame (60 FPS)
    static constexpr float STOP_SPEED   = 5.f;    // di bawah ini dianggap berhenti
    static constexpr float WALL_DAMPING = 0.9f;   // kehilangan energi saat kena dinding
};