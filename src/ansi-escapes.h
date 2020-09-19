/** @file
 * Funkcje obsługujące konsolę za pomocą ANSI escape codes
 *
 * @author Szymon Frąckowiak
 * @date 17.05.2020
 */

#ifndef GAMMA_ANSI_ESCAPES_H
#define GAMMA_ANSI_ESCAPES_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdint.h>
#include <inttypes.h>

/** @brief Włącza specjalny tryb pracy terminala.
 * Pozwala na wczytywanie klawiszy bez ich wyświetlania oraz
 * bez wciskania klawicza enter.
 */
void enable_raw_mode();

/** @brief Wyłącza specjalny tryb terminala. */
void disable_raw_mode();

/** @brief Pokazuje kursor (jeśli był ukryty). */
void enable_cursor();

/** @brief Ukrywa kursor. */
void hide_cursor();

/** @brief Czyści konsolę. */
void clear_console();

/** @brief Ustawia kolor dla pola gracza na planszy. */
void player_field();

/** @brief Ustawia kolor kursora na planszy. */
void cursor_field();

/** @brief Ustawia kolor obramowania planszy. */
void frame_colour();

/** @brief Ustawia pierwszy kolor pola. */
void field_colour_one();

/** @brief Ustawia drugi kolor pola. */
void field_colour_two();

/** @brief Ustawia pogrubienie tekstu. */
void bold();

/** @brief Usuwa wszystkie ustawienia graficzne. */
void reset_colour();

#endif //GAMMA_ANSI_ESCAPES_H
