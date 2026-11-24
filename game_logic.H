#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Bola.hpp"
#include "Lubang.hpp"
#include "Koordinat.hpp"

enum class GameState {
    Aiming,
    BallsMoving,
    GameOver
};

class GameLogic {
public:
    GameLogic(const std::vector<Lubang>& pockets, Koordinat cueStartPos);

    void onShotTaken();
    void update(std::vector<Bola>& balls, float dt);

    GameState getState() const { return state_; }
    int  getCurrentPlayer() const { return currentPlayer_; }
    int  getScore(int player) const { return scores_[player - 1]; }
    int  getWinner() const { return winner_; }
    bool wasFoul() const { return lastFoul_; }

    static bool allBallsStopped(const std::vector<Bola>& balls);

private:
    void checkPockets(std::vector<Bola>& balls);
    void endTurn(std::vector<Bola>& balls);
    void switchPlayer();

    std::vector<Lubang> pockets_;
    Koordinat cueStartPos_;

    GameState state_ = GameState::Aiming;
    int currentPlayer_ = 1;
    int scores_[2] = {0, 0};
    int winner_ = 0;

    int  pocketedThisTurn_ = 0;
    bool cuePocketedThisTurn_ = false;
    bool lastFoul_ = false;
};