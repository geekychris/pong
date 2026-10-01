#include "terminal.hpp"

#include <sys/ioctl.h>
#include <unistd.h>

#include <cstdio>

namespace term {

RawMode::RawMode() {
    tcgetattr(STDIN_FILENO, &origTermios_);
    struct termios raw = origTermios_;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= CS8;
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

RawMode::~RawMode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &origTermios_); }

void hideCursor() { std::fputs("\x1b[?25l", stdout); }
void showCursor() { std::fputs("\x1b[?25h", stdout); }
void clearScreen() { std::fputs("\x1b[2J\x1b[H", stdout); }
void moveCursorHome() { std::fputs("\x1b[H", stdout); }

void moveCursor(int row, int col) { std::printf("\x1b[%d;%dH", row, col); }

const char* fg(Color c) {
    switch (c) {
        case Color::Default: return "\x1b[39m";
        case Color::Red: return "\x1b[31m";
        case Color::Green: return "\x1b[32m";
        case Color::Yellow: return "\x1b[33m";
        case Color::Blue: return "\x1b[34m";
        case Color::Magenta: return "\x1b[35m";
        case Color::Cyan: return "\x1b[36m";
        case Color::White: return "\x1b[37m";
        case Color::BrightRed: return "\x1b[91m";
        case Color::BrightGreen: return "\x1b[92m";
        case Color::BrightYellow: return "\x1b[93m";
        case Color::BrightBlue: return "\x1b[94m";
        case Color::BrightMagenta: return "\x1b[95m";
        case Color::BrightCyan: return "\x1b[96m";
        case Color::BrightWhite: return "\x1b[97m";
    }
    return "\x1b[39m";
}

const char* resetSeq() { return "\x1b[0m"; }

Key readKey() {
    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n <= 0) return Key::None;

    if (c == '\x1b') {
        unsigned char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) <= 0) return Key::Esc;
        if (read(STDIN_FILENO, &seq[1], 1) <= 0) return Key::Esc;
        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return Key::Up;
                case 'B': return Key::Down;
                case 'C': return Key::Right;
                case 'D': return Key::Left;
                default: break;
            }
        }
        return Key::Esc;
    }

    switch (c) {
        case 'w': case 'W': return Key::W;
        case 's': case 'S': return Key::S;
        case 'q': case 'Q': return Key::Q;
        case 'p': case 'P': return Key::P;
        case 'r': case 'R': return Key::R;
        case '\r': case '\n': return Key::Enter;
        case ' ': return Key::Space;
        case '1': return Key::Digit1;
        case '2': return Key::Digit2;
        default: break;
    }
    return Key::Other;
}

int terminalWidth() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) return ws.ws_col;
    return 80;
}

int terminalHeight() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_row > 0) return ws.ws_row;
    return 24;
}

}  // namespace term
