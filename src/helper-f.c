#include "helper-f.h"

typedef enum {
    DIR_START = 0, LEFT = 0, TOP, RIGHT, BOTTOM, DIR_END
} dir_t;

player_t *get_player(gamma_t *g, uint32_t p) {
    return (g->players) + p - 1;
}

field_t *get_field(gamma_t *g, uint32_t x, uint32_t y) {
    return (g->board) + (g->board_width) * y + x;
}

static bool get_position(gamma_t *g, uint32_t x_field, uint32_t y_field, dir_t dir, uint32_t *x, uint32_t *y) {
    if (dir == LEFT && x_field > 0) {
        *x = x_field - 1;
        *y = y_field;
        return true;
    }
    if (dir == TOP && y_field > 0) {
        *x = x_field;
        *y = y_field - 1;
        return true;
    }
    if (dir == RIGHT && x_field < g->board_width - 1) {
        *x = x_field + 1;
        *y = y_field;
        return true;
    }
    if (dir == BOTTOM && y_field < g->board_height - 1) {
        *x = x_field;
        *y = y_field + 1;
        return true;
    }
    return false;
}

static field_t *find_area_root(field_t *a) {
    if (a->parent == NULL) {
        return a;
    }

    a->parent = find_area_root(a->parent);
    return a->parent;
}

static void union_areas(field_t *a, field_t *b) {
    field_t *a_root = find_area_root(a);
    field_t *b_root = find_area_root(b);

    if (a_root != b_root) {
        a->player->busy_areas -= 1;
    }

    if (a_root->rank > b_root->rank) {
        b_root->parent = a_root;
    } else if (a_root->rank < b_root->rank) {
        a_root->parent = b_root;
    } else if (a_root != b_root) {
        b_root->parent = a_root;
        a_root->rank += 1;
    }
}

void merge_with_areas(gamma_t *g, uint32_t field_x, uint32_t field_y) {
    field_t *f = get_field(g, field_x, field_y);
    f->parent = NULL;
    f->rank = 0;
    uint32_t x, y;
    for (dir_t dir = DIR_START; dir < DIR_END; dir++) {
        if (!get_position(g, field_x, field_y, dir, &x, &y)) {
            continue;
        }

        field_t *field = get_field(g, x, y);

        if (field->player != f->player) {
            continue;
        }

        union_areas(field, f);
    }
}

uint8_t count_same_players(gamma_t *g, uint32_t field_x, uint32_t field_y, player_t *p,
                           uint8_t start) {
    uint8_t count = 0;
    uint32_t x, y;
    for (dir_t dir = DIR_START; dir < DIR_END; dir++) {
        if (start > dir) {
            continue;
        }

        if (!get_position(g, field_x, field_y, dir, &x, &y)) {
            continue;
        }

        field_t *field = get_field(g, x, y);

        if (field->player != p) {
            continue;
        }

        count++;
    }
    return count;
}

void add_always_free(gamma_t *g, uint32_t field_x, uint32_t field_y) {
    field_t *f = get_field(g, field_x, field_y);

    uint32_t x, y;
    for (dir_t dir = DIR_START; dir < DIR_END; dir++) {
        if (!get_position(g, field_x, field_y, dir, &x, &y)) {
            continue;
        }

        field_t *field = get_field(g, x, y);

        if (!field->player &&
            count_same_players(g, x, y, f->player, 0) == 1) {
            f->player->always_free_fields += 1;
        } else if (field->player &&
                   count_same_players(g, field_x, field_y, field->player, dir + 1) == 0) {
            field->player->always_free_fields -= 1;
        }

    }
}

static void remove_always_free(gamma_t *g, uint32_t field_x, uint32_t field_y,
                               player_t *p) {
    uint32_t x, y;
    for (dir_t dir = DIR_START; dir < DIR_END; dir++) {
        if (!get_position(g, field_x, field_y, dir, &x, &y)) {
            continue;
        }

        field_t *field = get_field(g, x, y);

        if (!field->player &&
            count_same_players(g, x, y, p, 0) == 0) {
            p->always_free_fields -= 1;
        } else if (field->player &&
                   count_same_players(g, field_x, field_y, field->player, dir + 1) == 0) {
            field->player->always_free_fields += 1;
        }

    }
}

static void update_parent_dfs(gamma_t *g, uint32_t field_x, uint32_t field_y,
                              field_t *new_parent, uint64_t update_time) {
    field_t *f = get_field(g, field_x, field_y);

    if (f == new_parent) {
        f->parent = NULL;
    } else {
        f->parent = new_parent;
    }

    f->last_updated = update_time;

    uint32_t x, y;
    for (dir_t dir = DIR_START; dir < DIR_END; dir++) {
        if (!get_position(g, field_x, field_y, dir, &x, &y)) {
            continue;
        }

        field_t *field = get_field(g, x, y);

        if (field->player != f->player || field->last_updated == update_time) {
            continue;
        }

        update_parent_dfs(g, x, y, new_parent, update_time);

    }
}

void remove_field(gamma_t *g, uint32_t field_x, uint32_t field_y) {
    field_t *f = get_field(g, field_x, field_y);
    f->player->busy_areas -= 1;
    f->player->busy_fields -= 1;
    g->all_free_fields += 1;

    player_t *p = f->player;

    f->player = NULL;
    if (p->busy_fields == 0) {
        g->active_players -= 1;
    }

    remove_always_free(g, field_x, field_y, p);

    uint64_t update_time = g->last_update_time;
    g->last_update_time += 1;


    uint32_t x, y;
    for (dir_t dir = DIR_START; dir < DIR_END; dir++) {
        if (!get_position(g, field_x, field_y, dir, &x, &y)) {
            continue;
        }

        field_t *field = get_field(g, x, y);

        if (field->player != p || field->last_updated == update_time) {
            continue;
        }

        p->busy_areas += 1;
        update_parent_dfs(g, x, y, field, update_time);

    }
}

uint32_t get_field_size(uint32_t n) {
    if(n == 0) {
        return 1;
    }
    uint32_t result = 0;
    while (n > 0) {
        result++;
        n /= 10;
    }
    return result;
}

bool gm_necessary(gamma_t *g, uint32_t player) {
    player_t *p = get_player(g, player);

    return !(p->golden_move_used == true || g->active_players == 0 ||
             g->number_of_players < 2);
}

bool gm_field_possible(gamma_t *g, uint32_t player, uint32_t x, uint32_t y) {
    field_t *f = get_field(g, x, y);
    player_t *p = get_player(g, player);

    if (f->player == NULL || f->player == p) {
        return false;
    }

    if (p->busy_areas == g->max_areas && count_same_players(g, x, y, p, 0) == 0) {
        return false;
    }

    player_t *c_p = f->player;

    remove_field(g, x, y);
    if (c_p->busy_areas > g->max_areas) {
        gamma_move(g, (uint32_t) (c_p->id), x, y);
        return false;
    }

    gamma_move(g, (uint32_t) (c_p->id), x, y);
    return true;
}