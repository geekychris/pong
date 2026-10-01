#include <chrono>
#include <cmath>
#include <cstdio>
#include <string>
#include <thread>

#include "game.hpp"
#include "terminal.hpp"

namespace {

using pong::PongGame;
using pong::Side;

enum class Mode { VsAi, TwoPlayer };

constexpr double kTickSeconds = 1.0 / 30.0;
constexpr double kPaddleStep = 1.5;  // cells moved per keypress event

void appendCentered(std::string& out, const std::string& text, int width) {
    int pad = (width - static_cast<int>(text.size())) / 2;
    if (pad > 0) out.append(pad, ' ');
    out += text;
}

// Blocks (polling, non-busy) until the player picks a mode, or returns
// std::nullopt-like via the quit flag if they ask to quit from the menu.
Mode showMenu(bool& quit) {
    std::string frame;
    frame += "\x1b[2J\x1b[H";
    frame += term::fg(term::Color::BrightCyan);
    frame += "\n";
    appendCentered(frame, "=== TERMINAL PONG ===", pong::kBoardWidth + 2);
    frame += "\n\n";
    frame += term::resetSeq();
    frame += term::fg(term::Color::BrightWhite);
    appendCentered(frame, "1) Single Player (vs AI)", pong::kBoardWidth + 2);
    frame += "\n";
    appendCentered(frame, "2) Two Player (W/S  vs  Up/Down)", pong::kBoardWidth + 2);
    frame += "\n";
    appendCentered(frame, "Q) Quit", pong::kBoardWidth + 2);
    frame += "\n";
    frame += term::resetSeq();
    std::fwrite(frame.data(), 1, frame.size(), stdout);
    std::fflush(stdout);

    while (true) {
        term::Key key = term::readKey();
        if (key == term::Key::Digit1) return Mode::VsAi;
        if (key == term::Key::Digit2) return Mode::TwoPlayer;
        if (key == term::Key::Q) {
            quit = true;
            return Mode::VsAi;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

std::string renderFrame(const PongGame& game, bool paused, Mode mode) {
    using pong::kBoardHeight;
    using pong::kBoardWidth;
    using pong::kLeftPaddleX;
    using pong::kPaddleHeight;
    using pong::kRightPaddleX;

    const char* kBorder = term::fg(term::Color::Cyan);
    const char* kLeftColor = term::fg(term::Color::BrightGreen);
    const char* kRightColor = term::fg(term::Color::BrightMagenta);
    const char* kBallColor = term::fg(term::Color::BrightYellow);
    const char* kNetColor = term::fg(term::Color::White);
    const char* kReset = term::resetSeq();

    std::string out;
    out += "\x1b[H";  // move to home without clearing, to minimize flicker

    // Score line.
    std::string scoreLine = std::string(kLeftColor) + "LEFT: " + std::to_string(game.score(Side::Left)) +
                             kReset + "    " +
                             (mode == Mode::VsAi ? "(vs AI)" : "(2 Player)") + "    " + kRightColor +
                             "RIGHT: " + std::to_string(game.score(Side::Right)) + kReset;
    out += scoreLine;
    out += "   [W/S] left  [Up/Down] right  [P] pause  [Q] quit   \n";

    // Top border.
    out += kBorder;
    out += "+" + std::string(kBoardWidth, '-') + "+";
    out += kReset;
    out += "\n";

    int leftTop = static_cast<int>(std::lround(game.paddleY(Side::Left)));
    int rightTop = static_cast<int>(std::lround(game.paddleY(Side::Right)));
    int ballCol = static_cast<int>(std::lround(game.ball().x));
    int ballRow = static_cast<int>(std::lround(game.ball().y));

    for (int y = 0; y < kBoardHeight; ++y) {
        out += kBorder;
        out += "|";
        out += kReset;
        for (int x = 0; x < kBoardWidth; ++x) {
            bool isLeftPaddle = x == kLeftPaddleX && y >= leftTop && y < leftTop + kPaddleHeight;
            bool isRightPaddle = x == kRightPaddleX && y >= rightTop && y < rightTop + kPaddleHeight;
            bool isBall = x == ballCol && y == ballRow;
            bool isNet = x == kBoardWidth / 2 && (y % 2 == 0);

            if (isLeftPaddle) {
                out += kLeftColor;
                out += "#";
                out += kReset;
            } else if (isRightPaddle) {
                out += kRightColor;
                out += "#";
                out += kReset;
            } else if (isBall) {
                out += kBallColor;
                out += "O";
                out += kReset;
            } else if (isNet) {
                out += kNetColor;
                out += ":";
                out += kReset;
            } else {
                out += " ";
            }
        }
        out += kBorder;
        out += "|";
        out += kReset;
        out += "\n";
    }

    out += kBorder;
    out += "+" + std::string(kBoardWidth, '-') + "+";
    out += kReset;
    out += "\n";

    if (paused) {
        out += term::fg(term::Color::BrightYellow);
        appendCentered(out, "-- PAUSED --", kBoardWidth + 2);
        out += kReset;
        out += "\n";
    } else if (game.isGameOver()) {
        const char* winnerColor = game.winner() == Side::Left ? kLeftColor : kRightColor;
        std::string msg = (game.winner() == Side::Left ? std::string("LEFT") : std::string("RIGHT")) +
                           " WINS!  Press R to play again, Q to quit.";
        out += winnerColor;
        appendCentered(out, msg, kBoardWidth + 2);
        out += kReset;
        out += "\n";
    } else {
        out += std::string(kBoardWidth + 2, ' ');
        out += "\n";
    }

    return out;
}

}  // namespace

int main() {
    term::RawMode rawMode;
    term::hideCursor();
    term::clearScreen();

    bool quit = false;
    Mode mode = showMenu(quit);

    PongGame game;
    game.setAiEnabled(mode == Mode::VsAi);
    game.resetMatch();

    bool paused = false;
    term::clearScreen();

    while (!quit) {
        term::Key key;
        while ((key = term::readKey()) != term::Key::None) {
            if (key == term::Key::Q) {
                quit = true;
                break;
            }
            if (game.isGameOver()) {
                if (key == term::Key::R) {
                    game.resetMatch();
                    paused = false;
                }
                continue;
            }
            switch (key) {
                case term::Key::P:
                    paused = !paused;
                    break;
                case term::Key::W:
                    game.movePaddle(Side::Left, -kPaddleStep);
                    break;
                case term::Key::S:
                    game.movePaddle(Side::Left, kPaddleStep);
                    break;
                case term::Key::Up:
                    if (!game.aiEnabled()) game.movePaddle(Side::Right, -kPaddleStep);
                    break;
                case term::Key::Down:
                    if (!game.aiEnabled()) game.movePaddle(Side::Right, kPaddleStep);
                    break;
                default:
                    break;
            }
        }

        if (quit) break;

        if (!paused && !game.isGameOver()) {
            game.update(kTickSeconds);
        }

        std::string frame = renderFrame(game, paused, mode);
        std::fwrite(frame.data(), 1, frame.size(), stdout);
        std::fflush(stdout);

        std::this_thread::sleep_for(std::chrono::duration<double>(kTickSeconds));
    }

    term::showCursor();
    term::clearScreen();
    std::fputs("Thanks for playing Terminal Pong!\n", stdout);
    return 0;
}
