// Unit tests for the headless pong::PongGame logic. Built with
// PONG_ENABLE_TEST_HOOKS so the game's state can be set directly instead of
// having to be driven into position through play.
#include <cassert>
#include <cmath>
#include <cstdio>

#include "game.hpp"

using pong::PongGame;
using pong::Side;

namespace {

int g_failures = 0;

void check(bool condition, const char* what) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", what);
        g_failures++;
    } else {
        std::printf("PASS: %s\n", what);
    }
}

void testInitialState() {
    PongGame game;
    check(game.score(Side::Left) == 0, "initial left score is 0");
    check(game.score(Side::Right) == 0, "initial right score is 0");
    check(!game.isGameOver(), "game is not over initially");
    check(game.ball().vx != 0.0, "ball has horizontal velocity after construction");
}

void testTopWallBounce() {
    PongGame game;
    game.setBallForTesting(pong::kBoardWidth / 2.0, 0.2, 0.0, -10.0);
    game.update(0.05);  // would move y to -0.3 without the wall clamp
    check(game.ball().y >= 0.0, "ball stays within top wall");
    check(game.ball().vy > 0.0, "ball vy flips to positive after hitting top wall");
}

void testBottomWallBounce() {
    PongGame game;
    double bottom = pong::kBoardHeight - 1.0;
    game.setBallForTesting(pong::kBoardWidth / 2.0, bottom - 0.2, 0.0, 10.0);
    game.update(0.05);
    check(game.ball().y <= bottom, "ball stays within bottom wall");
    check(game.ball().vy < 0.0, "ball vy flips to negative after hitting bottom wall");
}

void testLeftPaddleCollision() {
    PongGame game;
    double paddleTop = 8.0;
    game.setPaddleForTesting(Side::Left, paddleTop);
    double paddleMidY = paddleTop + pong::kPaddleHeight / 2.0;
    game.setBallForTesting(pong::kLeftPaddleX + 1.0, paddleMidY, -20.0, 0.0);
    int scoreBefore = game.score(Side::Right);
    game.update(0.01);
    check(game.ball().vx > 0.0, "ball reflects off left paddle (vx becomes positive)");
    check(game.score(Side::Right) == scoreBefore, "no score change on a successful paddle hit");
}

void testRightPaddleCollision() {
    PongGame game;
    game.setAiEnabled(false);
    double paddleTop = 3.0;
    game.setPaddleForTesting(Side::Right, paddleTop);
    double paddleMidY = paddleTop + pong::kPaddleHeight / 2.0;
    game.setBallForTesting(pong::kRightPaddleX - 1.0, paddleMidY, 20.0, 0.0);
    int scoreBefore = game.score(Side::Left);
    game.update(0.01);
    check(game.ball().vx < 0.0, "ball reflects off right paddle (vx becomes negative)");
    check(game.score(Side::Left) == scoreBefore, "no score change on a successful paddle hit");
}

void testLeftConcedesPoint() {
    PongGame game;
    // Keep the left paddle far away from the ball's row so it can't block.
    game.setPaddleForTesting(Side::Left, 0.0);
    game.setBallForTesting(0.3, pong::kBoardHeight - 1.0, -50.0, 0.0);
    int rightBefore = game.score(Side::Right);
    game.update(0.1);  // pushes ball.x well below 0
    check(game.score(Side::Right) == rightBefore + 1, "right scores when ball passes the left paddle");
    check(!game.isGameOver(), "a single point is not game over");
    check(game.ball().x > 0.0 && game.ball().x < pong::kBoardWidth, "ball re-centers after a score");
}

void testRightConcedesPoint() {
    PongGame game;
    game.setAiEnabled(false);
    game.setPaddleForTesting(Side::Right, 0.0);
    game.setBallForTesting(pong::kBoardWidth - 1.3, pong::kBoardHeight - 1.0, 50.0, 0.0);
    int leftBefore = game.score(Side::Left);
    game.update(0.1);
    check(game.score(Side::Left) == leftBefore + 1, "left scores when ball passes the right paddle");
}

void testGameOverAndWinner() {
    PongGame game;
    game.setScoreForTesting(Side::Left, pong::kWinningScore - 1);
    check(!game.isGameOver(), "not yet game over one point before the winning score");
    game.setScoreForTesting(Side::Left, pong::kWinningScore);
    check(game.isGameOver(), "game over once a side reaches the winning score");
    check(game.winner() == Side::Left, "left is reported as the winner");
}

void testResetMatchClearsScores() {
    PongGame game;
    game.setScoreForTesting(Side::Left, 3);
    game.setScoreForTesting(Side::Right, 4);
    game.resetMatch();
    check(game.score(Side::Left) == 0 && game.score(Side::Right) == 0, "resetMatch clears both scores");
    check(!game.isGameOver(), "resetMatch clears game-over state");
}

void testMovePaddleClampsToBoard() {
    PongGame game;
    game.movePaddle(Side::Left, -1000.0);
    check(game.paddleY(Side::Left) == 0.0, "paddle does not move above the top of the board");
    game.movePaddle(Side::Left, 1000.0);
    check(game.paddleY(Side::Left) == pong::kBoardHeight - pong::kPaddleHeight,
          "paddle does not move below the bottom of the board");
}

void testAiTracksBall() {
    PongGame game;
    game.setAiEnabled(true);
    game.setPaddleForTesting(Side::Right, pong::kBoardHeight / 2.0 - pong::kPaddleHeight / 2.0);
    // Put the ball near the top, far from the paddle's current position.
    game.setBallForTesting(pong::kBoardWidth / 2.0, 0.0, 0.0, 0.0);
    double before = game.paddleY(Side::Right);
    game.update(0.05);
    check(game.paddleY(Side::Right) < before, "AI paddle moves towards the ball");
}

}  // namespace

int main() {
    testInitialState();
    testTopWallBounce();
    testBottomWallBounce();
    testLeftPaddleCollision();
    testRightPaddleCollision();
    testLeftConcedesPoint();
    testRightConcedesPoint();
    testGameOverAndWinner();
    testResetMatchClearsScores();
    testMovePaddleClampsToBoard();
    testAiTracksBall();

    if (g_failures > 0) {
        std::fprintf(stderr, "\n%d test(s) failed.\n", g_failures);
        return 1;
    }
    std::printf("\nAll tests passed.\n");
    return 0;
}
