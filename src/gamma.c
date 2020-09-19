#include "gamma.h"

gamma_t *gamma_new(uint32_t width, uint32_t height,
                   uint32_t players, uint32_t areas) {
    if (width < 1 || height < 1 || players < 1 || areas < 1) {
        return NULL;
    }
    gamma_t *g = (gamma_t *) calloc(1, sizeof(gamma_t));
    if (!g) {
        return NULL;
    }
    player_t *new_players = (player_t *) calloc(players, sizeof(player_t));
    if (!new_players) {
        free(g);
        return NULL;
    }
    field_t *new_board = (field_t *) calloc((uint64_t) width * height, sizeof(field_t));
    if (!new_board) {
        free(g);
        free(new_players);
        return NULL;
    }
    for (uint32_t i = 0; i < players; i++) {
        new_players[i].id = 1 + i;
    }
    for (uint64_t i = 0; i < (uint64_t) width * height; i++) {
        (new_board + i)->player = NULL;
    }
    g->board_width = width;
    g->board_height = height;
    g->number_of_players = players;
    g->active_players = 0;
    g->max_areas = areas;
    g->last_update_time = 1;
    g->all_free_fields = (uint64_t) width * height;
    g->players = new_players;
    g->board = new_board;
    return g;
}

void gamma_delete(gamma_t *g) {
    if (!g) {
        return;
    }
    free(g->players);
    free(g->board);
    free(g);
}

bool gamma_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    if (g == NULL || player < 1 || player > g->number_of_players ||
        x >= g->board_width || y >= g->board_height) {
        return false;
    }
    field_t *f = get_field(g, x, y);
    if (f->player != NULL) {
        return false;
    }

    player_t *p = get_player(g, player);

    if (p->busy_areas >= g->max_areas &&
        count_same_players(g, x, y, p, 0) == 0) {
        return false;
    }

    f->player = p;
    if (p->busy_fields == 0) {
        g->active_players += 1;
    }
    p->busy_fields += 1;
    p->busy_areas += 1;
    add_always_free(g, x, y);
    merge_with_areas(g, x, y);
    g->all_free_fields -= 1;

    return true;
}

bool gamma_golden_move(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    if (g == NULL || player < 1 || player > g->number_of_players ||
        x >= g->board_width || y >= g->board_height) {
        return false;
    }

    if (!gm_necessary(g, player)) {
        return false;
    }

    if (!gm_field_possible(g, player, x, y)) {
        return false;
    }

    remove_field(g, x, y);
    gamma_move(g, player, x, y);

    get_player(g, player)->golden_move_used = true;

    return true;
}

uint64_t gamma_busy_fields(gamma_t *g, uint32_t player) {
    if (g == NULL || player < 1 || player > g->number_of_players) {
        return 0;
    }
    return get_player(g, player)->busy_fields;
}

uint64_t gamma_free_fields(gamma_t *g, uint32_t player) {
    if (g == NULL || player < 1 || player > g->number_of_players) {
        return 0;
    }
    player_t *p = get_player(g, player);
    if (p->busy_areas >= g->max_areas) {
        return p->always_free_fields;
    }
    return g->all_free_fields;
}

bool gamma_golden_possible(gamma_t *g, uint32_t player) {
    if (g == NULL || player < 1 || player > g->number_of_players) {
        return false;
    }

    if (!gm_necessary(g, player)) {
        return false;
    }

    for (uint32_t y = 0; y < g->board_height; y++) {
        for (uint32_t x = 0; x < g->board_width; x++) {
            if (gm_field_possible(g, player, x, y)) {
                return true;
            }
        }
    }
    return false;
}

char *gamma_board(gamma_t *g) {
    if (g == NULL) {
        return NULL;
    }

    uint32_t field_width = get_field_size(g->number_of_players);
    size_t board_size = (g->board_width * field_width + 1) * g->board_height + 1;
    char *board_display = malloc(board_size * sizeof(char));

    if (board_display == NULL) {
        return NULL;
    }

    uint64_t i = 0;
    for (uint32_t y = (g->board_height); y > 0; y--) {
        for (uint32_t x = 0; x < g->board_width; x++) {
            if (get_field(g, x, y - 1)->player == NULL) {
                for (uint32_t j = 0; j < field_width - 1; j++) {
                    board_display[i] = '_';
                    i++;
                }
                board_display[i] = '.';
                i++;
            } else {
                uint32_t player_number = get_field(g, x, y - 1)->player->id;
                uint32_t digits_number = get_field_size(player_number);

                for (uint32_t j = 0; j < field_width - digits_number; j++) {
                    board_display[i] = '_';
                    i++;
                }

                char *string_int = malloc((digits_number + 1) * sizeof(char));

                if (string_int == NULL) {
                    free(board_display);
                    return NULL;
                }

                sprintf(string_int, "%u", player_number);

                for (uint32_t j = 0; j < digits_number; j++) {
                    board_display[i] = string_int[j];
                    i++;
                }
                free(string_int);
            }
        }
        board_display[i] = '\n';
        i++;
    }
    board_display[i] = '\0';
    return board_display;
}
