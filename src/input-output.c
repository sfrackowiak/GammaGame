#define _GNU_SOURCE
#define __STDC_FORMAT_MACROS

#include "input-output.h"

#define OK_CHAR "BImgbfqp"
#define OK_CHAR_SIZE 8
#define MAX_ARGS_NUMBER 4
#define ESC_CHAR 27
#define UP_ARROW 1001
#define DOWN_ARROW 1002
#define RIGHT_ARROW 1003
#define LEFT_ARROW 1004
#define SPACE 32
#define END 4
#define TEXT_WIDTH 40
#define TEXT_HIGHT 11
#define PLAYER_WIDTH 6
#define BUSY_WIDTH 11

static void error_msg(unsigned long line_number) {
    fprintf(stderr, "ERROR %lu\n", line_number);
}

static void ok_msg(unsigned long line_number) {
    printf("OK %lu\n", line_number);
}

static bool is_comment(const char *buffer) {
    if (buffer[0] == '#' || buffer[0] == '\n') {
        return true;
    }
    return false;
}

static line_t *process_line(char *buffer, ssize_t length) {
    if (length < 1 || isspace(buffer[0])) {
        return NULL;
    }
    bool correct = false;
    char tab[] = OK_CHAR;
    for (int i = 0; i < OK_CHAR_SIZE; i++) {
        if (buffer[0] == tab[i]) {
            correct = true;
            break;
        }
    }
    if (correct == false) {
        return NULL;
    }

    if (length > 2 && !isspace(buffer[1])) {
        return NULL;
    }

    for (ssize_t i = 1; i < length; i++) {
        char c = buffer[i];
        if (!isspace(c) && (c < '0' || c > '9')) {
            return NULL;
        }
    }

    line_t *l = (line_t *) calloc(1, sizeof(line_t));
    if (!l) {
        return NULL;
    }

    l->command = buffer[0];
    l->number_of_args = 0;

    char *string = buffer + 1;
    char *end_ptr;

    for (int i = 0; i < MAX_ARGS_NUMBER + 1; i++) {
        errno = 0;
        uint64_t number = strtoull(string, &end_ptr, 10);
        if (errno != 0) {
            free(l);
            return NULL;
        }
        if (string == end_ptr) {
            break;
        }
        if (i > MAX_ARGS_NUMBER - 1) {
            free(l);
            return NULL;
        }
        if (number > UINT32_MAX) {
            free(l);
            return NULL;
        }
        l->arg[i] = (uint32_t) number;
        l->number_of_args++;
        string = end_ptr;
    }

    return l;
}

char mode_selection(gamma_t **g, unsigned long *line_number) {
    char *buffer = NULL;
    size_t buffer_size = 0;
    ssize_t length;

    char selected = '0';

    while ((length = getline(&buffer, &buffer_size, stdin)) != -1) {
        *line_number += 1;
        if (length > 0 && is_comment(buffer)) {
            continue;
        }
        line_t *l = process_line(buffer, length);
        if (!l) {
            error_msg(*line_number);
            continue;
        }
        if ((l->command != 'B' && l->command != 'I') || l->number_of_args != 4) {
            error_msg(*line_number);
            free(l);
            continue;
        }
        *g = gamma_new(l->arg[0], l->arg[1], l->arg[2], l->arg[3]);
        if (!*g) {
            error_msg(*line_number);
            free(l);
            continue;
        }
        selected = l->command;
        free(l);
        break;
    }

    free(buffer);
    return selected;
}

static void result_bool(bool result) {
    if (result) {
        printf("1\n");
    } else {
        printf("0\n");
    }
}

static void result_uint64(uint64_t result) {
    printf("%" PRIu64 "\n", result);
}

static bool check_args_number(line_t *l, int correct_number, unsigned long line_number) {
    if (l->number_of_args != correct_number) {
        error_msg(line_number);
        return false;
    }
    return true;
}

void batch_mode(gamma_t *g, unsigned long *line_number) {
    ok_msg(*line_number);

    char *buffer = NULL;
    size_t buffer_size = 0;
    ssize_t length;

    while ((length = getline(&buffer, &buffer_size, stdin)) != -1) {
        *line_number += 1;
        if (length > 0 && is_comment(buffer)) {
            continue;
        }
        line_t *l = process_line(buffer, length);
        if (!l) {
            error_msg(*line_number);
            continue;
        }
        switch (l->command) {
            case 'm':
                if (!check_args_number(l, 3, *line_number)) {
                    break;
                }
                result_bool(gamma_move(g, l->arg[0], l->arg[1], l->arg[2]));
                break;
            case 'g':
                if (!check_args_number(l, 3, *line_number)) {
                    break;
                }
                result_bool(gamma_golden_move(g, l->arg[0], l->arg[1], l->arg[2]));
                break;
            case 'b':
                if (!check_args_number(l, 1, *line_number)) {
                    break;
                }
                result_uint64(gamma_busy_fields(g, l->arg[0]));
                break;
            case 'f':
                if (!check_args_number(l, 1, *line_number)) {
                    break;
                }
                result_uint64(gamma_free_fields(g, l->arg[0]));
                break;
            case 'q':
                if (!check_args_number(l, 1, *line_number)) {
                    break;
                }
                result_bool(gamma_golden_possible(g, l->arg[0]));
                break;
            case 'p':
                if (!check_args_number(l, 0, *line_number)) {
                    break;
                }
                char *board = gamma_board(g);
                printf("%s", board);
                free(board);
                break;
            default:
                error_msg(*line_number);
        }
        free(l);
    }
    free(buffer);
}

static bool check_console_size(gamma_t *g) {
    struct winsize size;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

    uint32_t field_size = get_field_size(g->number_of_players);
    if (field_size % 2 == 1) {
        field_size++;
    }

    uint32_t game_width = g->board_width * field_size + 4;
    if (TEXT_WIDTH > game_width) {
        game_width = TEXT_WIDTH;
    }

    uint32_t game_height = g->board_height + 2 + TEXT_HIGHT;

    if (size.ws_col < game_width || size.ws_row < game_height) {
        return false;
    }
    return true;
}

static void print_horizontal_frame(uint32_t width, uint32_t field_width) {
    frame_colour();
    uint32_t total_width = width * field_width + 4;
    if (field_width % 2 == 1) {
        total_width += width;
    }

    for (uint32_t i = 0; i < total_width; i++) {
        printf(" ");
    }

    printf("\n");
}

static void print_board(gamma_t *g, uint32_t cursor_x, uint32_t cursor_y, uint32_t player) {
    char *board = gamma_board(g);

    uint32_t field_width = get_field_size(g->number_of_players);

    print_horizontal_frame(g->board_width, field_width);

    uint64_t i = 0;
    for (uint32_t y = g->board_height; y > 0; y--) {
        frame_colour();
        printf("  ");
        for (uint32_t x = 0; x < g->board_width; x++) {
            if (x % 2 == (y - 1) % 2) {
                field_colour_one();
            } else {
                field_colour_two();
            }

            player_t *field_player = get_field(g, x, y - 1)->player;
            if (field_player != NULL && field_player->id == player) {
                player_field();
            }
            if (player != 0 && x == cursor_x && y - 1 == cursor_y) {
                cursor_field();
            }

            for (uint32_t j = 0; j < field_width; j++) {
                if (field_width % 2 == 1) {
                    printf(" ");
                }
                if (board[i] == '_' || board[i] == '.') {
                    printf(" ");
                } else {
                    printf("%c", board[i]);
                }
                i++;
            }

            reset_colour();

            if (x == g->board_width - 1) {
                frame_colour();
                printf("  ");
            }
        }
        printf("\n");
        i++;
    }

    print_horizontal_frame(g->board_width, field_width);

    reset_colour();
    free(board);
}

static void print_game(gamma_t *g, uint32_t cursor_x, uint32_t cursor_y, uint32_t player) {
    clear_console();

    print_board(g, cursor_x, cursor_y, player);

    bold();
    printf("PLAYER %"PRIu32"\n", player);
    reset_colour();
    printf("> number of free fields: %"PRIu64"\n", gamma_free_fields(g, player));
    printf("> number of busy fields: %"PRIu64"\n", gamma_busy_fields(g, player));
    if (gamma_golden_possible(g, player)) {
        printf("> ");
        bold();
        printf("golden move possible\n");
        reset_colour();
    }
    else {
        printf("\n");
    }

    printf("\nUse arrows to move around the board.\n"
           "SPACE   - make a move.\n"
           "G       - make a golden move.\n"
           "C       - give up a move.\n"
           "Ctrl+D  - quit the game.\n");
}

static void end_of_game(gamma_t *g) {
    clear_console();
    print_board(g, 0, 0, 0);
    bold();
    printf("Game over!\n");

    uint64_t best_score = 0;

    printf("PLAYER | BUSY FIELDS\n");
    reset_colour();

    for(uint32_t i = 1; i <= g->number_of_players; i++) {
        if(gamma_busy_fields(g, i) > best_score) {
            best_score = gamma_busy_fields(g, i);
        }
    }

    for (uint32_t i = 1; i <= g->number_of_players; i++) {
        if(gamma_busy_fields(g, i) == best_score) {
            frame_colour();
        }
        printf("%"PRIu32, i);
        for(int j = get_field_size(i); j < PLAYER_WIDTH; j++) {
            printf(" ");
        }
        printf(" | ");
        for(int j = get_field_size(gamma_busy_fields(g, i)); j < BUSY_WIDTH; j++) {
            printf(" ");
        }
        printf("%"PRIu64"\n", gamma_busy_fields(g, i));
        reset_colour();
    }
}

static int get_arrow(int c) {
    switch (c) {
        case 'A':
            return UP_ARROW;
        case 'B':
            return DOWN_ARROW;
        case 'C':
            return RIGHT_ARROW;
        case 'D':
            return LEFT_ARROW;
        default:
            return c;
    }
}

static int read_input() {
    int c = getchar();
    if (c == ESC_CHAR) {
        c = getchar();
        if (c != '[') {
            return c;
        }
        c = getchar();
        return get_arrow(c);
    }
    return c;
}

static int process_input(int input, gamma_t *g, uint32_t player, uint32_t *x, uint32_t *y) {
    switch (input) {
        case EOF:
        case END:
            return -1;
        case UP_ARROW:
            if (*y < g->board_height - 1) {
                *y += 1;
            }
            return 0;
        case DOWN_ARROW:
            if (*y > 0) {
                *y -= 1;
            }
            return 0;
        case RIGHT_ARROW:
            if (*x < g->board_width - 1) {
                *x += 1;
            }
            return 0;
        case LEFT_ARROW:
            if (*x > 0) {
                *x -= 1;
            }
            return 0;
        case SPACE:
            return gamma_move(g, player, *x, *y);
        case 'g':
        case 'G':
            return gamma_golden_move(g, player, *x, *y);
        case 'c':
        case 'C':
            return 1;
        default:
            return 0;
    }
}

void interactive_mode(gamma_t *g) {
    atexit(reset_colour);
    enable_raw_mode();
    hide_cursor();

    if (!check_console_size(g)) {
        clear_console();
        printf("The console window is too small!\n"
               "Resize it and press any key to start the game.\n"
               "Ctrl+D terminates the program.\n");
        while (!check_console_size(g)) {
            int c = read_input();
            if (c == END) {
                clear_console();
                return;
            }
        }
    }

    uint32_t player = 0;
    uint32_t unable_move = 0;

    while (unable_move < g->number_of_players) {
        player++;
        if (player > g->number_of_players) {
            player = 1;
        }
        if (gamma_free_fields(g, player) == 0 && !gamma_golden_possible(g, player)) {
            unable_move++;
            continue;
        }

        unable_move = 0;
        bool end_of_move = false;

        uint32_t x = (g->board_width - 1) / 2;
        uint32_t y = g->board_height / 2;

        while (!end_of_move) {
            print_game(g, x, y, player);
            int read = read_input();
            int result = process_input(read, g, player, &x, &y);
            if (result == -1) {
                end_of_game(g);
                return;
            }
            if (result == 1) {
                end_of_move = true;
            }
        }
    }
    end_of_game(g);
}
