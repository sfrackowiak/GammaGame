#include "ansi-escapes.h"

#define ESC "\x1b"

struct termios original;

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &original);
    atexit(disable_raw_mode);
    struct termios new = original;
    new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
}

void enable_cursor() {
    printf(ESC"[?25h");
}

void hide_cursor() {
    atexit(enable_cursor);
    printf(ESC"[?25l");
}

void clear_console() {
    printf(ESC"[H"ESC"[J");
}

void player_field() {
    printf(ESC"[42m");
}

void cursor_field() {
    printf(ESC"[7m");
}

void frame_colour() {
    printf(ESC"[43m");
}

void field_colour_one() {
    printf(ESC"[44m");
}

void field_colour_two() {
    printf(ESC"[41m");
}

void bold() {
    printf(ESC"[1m");
}

void reset_colour() {
    printf(ESC"[m");
}
