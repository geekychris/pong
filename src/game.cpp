#include "game.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace pong {

PongGame::PongGame()
    : leftY_((kBoardHeight - kPaddleHeight) / 2.0), rightY_((kBoardHeight - kPaddleHeight) / 2.0) {
    resetRound(Side::Left);
}

void PongGame::resetMatch() {
    leftScore_ = 0;
    rightScore_ = 0;
    leftY_ = (kBoardHeight - kPaddleHeight) / 2.0;
    rightY_ = (kBoardHeight - kPaddleHeight) / 2.0;
    resetRound(Side::Left);
}

void PongGame::resetRound(Side scoredAgainst) {
    ball_.x = kBoardWidth / 2.0;
    ball_.y = kBoardHeight / 2.0;
    // Serve towards whichever side just conceded, so they must respond immediately.
    double dir = (scoredAgainst == Side::Left) ? -1.0 : 1.0;
    ball_.vx = kInitialBallSpeed * dir;
    double vySign = (std::rand() % 2 == 0) ? 1.0 : -1.0;
    ball_.vy = vySign * kInitialBallSpeed * 0.35;
}

void PongGame::movePaddle(Side side, double deltaY) {
    double& y = (side == Side::Left) ? leftY_ : rightY_;
    y = std::clamp(y + deltaY, 0.0, static_cast<double>(kBoardHeight - kPaddleHeight));
}

void PongGame::updateAi(double dtSeconds) {
    double center = rightY_ + kPaddleHeight / 2.0;
    double diff = std::clamp(ball_.y - center, -kPaddleSpeed * dtSeconds, kPaddleSpeed * dtSeconds);
    rightY_ = std::clamp(rightY_ + diff, 0.0, static_cast<double>(kBoardHeight - kPaddleHeight));
}

void PongGame::handleCollisions() {
    // Left paddle: ball must be moving left and crossing the paddle's column.
    if (ball_.vx < 0 && ball_.x <= kLeftPaddleX + 1 && ball_.x >= kLeftPaddleX - 1) {
        if (ball_.y >= leftY_ - 0.5 && ball_.y <= leftY_ + kPaddleHeight - 0.5) {
            ball_.x = kLeftPaddleX + 1;
            double relative = (ball_.y - (leftY_ + kPaddleHeight / 2.0)) / (kPaddleHeight / 2.0);
            ball_.vy = relative * kMaxBallSpeed * 0.6;
            ball_.vx = std::min(std::abs(ball_.vx) * 1.08, kMaxBallSpeed);
        }
    }

    // Right paddle: ball must be moving right and crossing the paddle's column.
    if (ball_.vx > 0 && ball_.x >= kRightPaddleX - 1 && ball_.x <= kRightPaddleX + 1) {
        if (ball_.y >= rightY_ - 0.5 && ball_.y <= rightY_ + kPaddleHeight - 0.5) {
            ball_.x = kRightPaddleX - 1;
            double relative = (ball_.y - (rightY_ + kPaddleHeight / 2.0)) / (kPaddleHeight / 2.0);
            ball_.vy = relative * kMaxBallSpeed * 0.6;
            ball_.vx = -std::min(std::abs(ball_.vx) * 1.08, kMaxBallSpeed);
        }
    }
}

void PongGame::update(double dtSeconds) {
    if (isGameOver()) {
        return;
    }

    if (aiEnabled_) {
        updateAi(dtSeconds);
    }

    ball_.x += ball_.vx * dtSeconds;
    ball_.y += ball_.vy * dtSeconds;

    if (ball_.y <= 0.0) {
        ball_.y = 0.0;
        ball_.vy = -ball_.vy;
    } else if (ball_.y >= kBoardHeight - 1.0) {
        ball_.y = kBoardHeight - 1.0;
        ball_.vy = -ball_.vy;
    }

    handleCollisions();

    if (ball_.x < 0.0) {
        // Ball passed the left paddle: right player scores, left conceded.
        rightScore_++;
        resetRound(Side::Left);
    } else if (ball_.x > kBoardWidth - 1.0) {
        // Ball passed the right paddle: left player scores, right conceded.
        leftScore_++;
        resetRound(Side::Right);
    }
}

}  // namespace pong
