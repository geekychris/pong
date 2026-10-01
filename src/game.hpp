#pragma once

namespace pong {

constexpr int kBoardWidth = 78;
constexpr int kBoardHeight = 22;
constexpr int kPaddleHeight = 5;
constexpr int kWinningScore = 5;
constexpr int kLeftPaddleX = 2;
constexpr int kRightPaddleX = kBoardWidth - 3;

constexpr double kInitialBallSpeed = 26.0;  // cells per second
constexpr double kMaxBallSpeed = 60.0;
constexpr double kPaddleSpeed = 26.0;       // cells per second, used by the AI

enum class Side { Left, Right };

struct Ball {
    double x = 0.0;
    double y = 0.0;
    double vx = 0.0;
    double vy = 0.0;
};

// Core game state and physics for Pong. Deliberately has no terminal/IO
// dependencies so it can be driven headlessly in unit tests.
class PongGame {
public:
    PongGame();

    // Resets scores to zero and starts a fresh round.
    void resetMatch();

    // Re-centers the ball and serves it towards `scoredAgainst`, keeping scores.
    void resetRound(Side scoredAgainst);

    // Advances ball physics, paddle AI (if enabled) and collisions by dtSeconds.
    void update(double dtSeconds);

    // Moves a paddle by deltaY cells, clamped to the playfield.
    void movePaddle(Side side, double deltaY);

    void setAiEnabled(bool enabled) { aiEnabled_ = enabled; }
    bool aiEnabled() const { return aiEnabled_; }

    double paddleY(Side side) const { return side == Side::Left ? leftY_ : rightY_; }
    int score(Side side) const { return side == Side::Left ? leftScore_ : rightScore_; }
    const Ball& ball() const { return ball_; }

    bool isGameOver() const { return leftScore_ >= kWinningScore || rightScore_ >= kWinningScore; }
    Side winner() const { return leftScore_ > rightScore_ ? Side::Left : Side::Right; }

#ifdef PONG_ENABLE_TEST_HOOKS
    // Test-only helpers for putting the game into a known state without
    // playing through it. Compiled in only when PONG_ENABLE_TEST_HOOKS is
    // defined (see tests/test_game.cpp), so they never ship in the real game.
    void setBallForTesting(double x, double y, double vx, double vy) {
        ball_.x = x;
        ball_.y = y;
        ball_.vx = vx;
        ball_.vy = vy;
    }
    void setPaddleForTesting(Side side, double y) {
        (side == Side::Left ? leftY_ : rightY_) = y;
    }
    void setScoreForTesting(Side side, int score) {
        (side == Side::Left ? leftScore_ : rightScore_) = score;
    }
#endif

private:
    void updateAi(double dtSeconds);
    void handleCollisions();

    double leftY_;
    double rightY_;
    int leftScore_ = 0;
    int rightScore_ = 0;
    Ball ball_;
    bool aiEnabled_ = true;
};

}  // namespace pong
