/** @file
 * Funkcje obsługujące grę w trybie wsadowym i interaktywnym.
 *
 * @author Szymon Frąckowiak
 * @date 17.05.2020
 */

#ifndef GAMMA_INPUT_OUTPUT_H
#define GAMMA_INPUT_OUTPUT_H

#include <inttypes.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include "helper-f.h"
#include "gamma.h"
#include "ansi-escapes.h"

/** @brief Struktura przechowująca informacje o poleceniu. */
typedef struct line {
    uint8_t number_of_args; ///< liczba argumentów polecenia
    char command;           ///< rodzaj polecenia
    uint32_t arg[4];        ///< argumenty polecenia
} line_t;

/** @brief Tworzy określoną przez użytkownika grę i zwraca jej tryb.
 * Funkcja przyjmuje jako argument podwójny wskaźnik na grę. Jeśli użytkownik
 * poprawnie określił grę to funkcja modyfikuję argument na wskaźnik do gry o
 * określonych parametrach. Zwracany jest oczekiwany przez użytkownika tryb gry:
 * B - tryb wsadowy, I - tryb interaktywny, 0 - użytkownik nie określił żadnej gry.
 * @param[in] g             - podwójny wskaźnik na strukturę przechowującą stan gry,
 * @param[in] line_number   - wskaźnik na numer linijki ostatniego polecenia.
 */
char mode_selection(gamma_t **g, unsigned long *line_number);

/** @brief Przejście do trybu wsadowego.
 * @param[in] g             - wskaźnik na strukturę przechowującą stan gry,
 * @param[in] line_number   - wskaźnik na numer linijki ostatniego polecenia.
 */
void batch_mode(gamma_t *g, unsigned long *line_number);

/** @brief Przejście do trybu interaktywnego.
 * @param[in] g             - wskaźnik na strukturę przechowującą stan gry.
 */
void interactive_mode(gamma_t *g);

#endif //GAMMA_INPUT_OUTPUT_H
