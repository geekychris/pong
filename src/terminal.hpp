#pragma once

#include <termios.h>

namespace term {

// RAII guard that puts the terminal into raw, non-blocking, non-echoing mode
// for the lifetime of the object and restores the previous settings on
// destruction.
class RawMode {
public:
    RawMode();
    ~RawMode();
    RawMode(const RawMode&) = delete;
    RawMode& operator=(const RawMode&) = delete;

private:
    struct termios origTermios_;
};

void hideCursor();
void showCursor();
void clearScreen();
void moveCursorHome();
void moveCursor(int row, int col);  // 1-based

enum class Color {
    Default,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    BrightRed,
    BrightGreen,
    BrightYellow,
    BrightBlue,
    BrightMagenta,
    BrightCyan,
    BrightWhite,
};

// Returns the ANSI SGR escape sequence that sets the foreground color.
const char* fg(Color c);
// Returns the ANSI SGR escape sequence that resets all attributes.
const char* resetSeq();

enum class Key {
    None,
    Up,
    Down,
    Left,
    Right,
    W,
    S,
    Q,
    P,
    R,
    Enter,
    Space,
    Digit1,
    Digit2,
    Esc,
    Other,
};

// Non-blocking: returns Key::None immediately if no input is waiting.
Key readKey();

int terminalWidth();
int terminalHeight();

}  // namespace term
