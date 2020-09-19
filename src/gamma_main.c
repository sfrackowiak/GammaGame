#include "gamma.h"
#include "input-output.h"

int main() {
    gamma_t *gamma_game = NULL;
    unsigned long line_number = 0;
    char mode = mode_selection(&gamma_game, &line_number);
    if (mode == 'B') {
        batch_mode(gamma_game, &line_number);
    } else if (mode == 'I') {
        interactive_mode(gamma_game);
    }
    gamma_delete(gamma_game);
}

