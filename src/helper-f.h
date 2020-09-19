/** @file
 * Funkcje i struktury pomocnicze dla interfejsu gry gamma
 *
 * @author Szymon Frąckowiak
 * @date 20.04.2020
 */

#ifndef HELPERF_H
#define HELPERF_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "gamma.h"

/** @brief Struktura przechowująca informacje o graczu.
 * Struktura przechowuje dane o liczbie zajętych obszarów i pól oraz informację
 * o wykorzystaniu złotego ruchu. Ponadto przechowywana jest informacja o
 * "zawsze" wolnych polach czyli o liczbie pól które gracz może zająć
 * (w standardowy sposób) nawet wtedy kiedy zajął już maksymalną liczbę
 * obszarów.
 */
typedef struct player {
    uint32_t id;                ///< numer gracza
    uint64_t busy_fields;       ///< liczba zajętych pól
    uint64_t busy_areas;        ///< liczba zajętych obszarów
    uint64_t always_free_fields;///< liczba "zawsze" możliwych do zajęcia pól
    bool golden_move_used;      ///< odpowiada czy gracz użył już złotego ruchu
} player_t;

/** @brief Struktura przechowująca informacje o polu planszy.
 * Domyślnie kiedy pole nie jest zajęte wskaźnik player jest NULL. Pole łączy
 * się z innymi polami zajętymi przez tego samego gracza w obszar za pomocą
 * struktury zbiorów rozłącznych (stąd zmienne parent i rank). Złoty ruch
 * (zazwyczaj) zmienia obszary w grze dlatego last_updated przechowuje
 * informację o ostatniej takiej zmianie.
 */
typedef struct field {
    player_t *player;     ///< wskaźnik na gracza zajmującego dane pole
    struct field *parent; ///< wskaźnik istotny dla łączenia obszarów
    uint64_t rank;        ///< liczba istotna dla łączenia obszarów
    uint64_t last_updated;///< "czas" ostatniej aktualizacji obszaru pola
} field_t;

/** @brief Struktura przechowująca stan gry.
 * Struktura przechowuje podstawowe informacje o grze takie jak: szerokość i
 * wysokość planszy, liczbę graczy, maksymalną liczbę obszarów oraz wskaźniki
 * na planszę i graczy. Dodatkowo przechowywane są informacje pomocnicze dla
 * funkcji takie jak: liczba aktywnych graczy (ile graczy ma chociaż jednego
 * pionka na planszy), ostatni "czas" aktualizacji obszarów (przy złotym
 * ruchu) oraz całkowitą liczbę niezajętych przez nikogo pól.
 */
typedef struct gamma {
    uint32_t board_width;      ///< szerokość planszy
    uint32_t board_height;     ///< wysokość planszy
    uint32_t number_of_players;///< liczba graczy
    uint32_t active_players;   ///< liczba różnych pionków na planszy
    uint32_t max_areas;        ///< maksymalna liczba obszarów
    uint64_t last_update_time; ///< ostatni "czas" aktualizacji obszarów
    uint64_t all_free_fields;  ///< całkowita liczba niezajętych pól
    player_t *players;         ///< tablica przechowująca wskaźniki na graczy
    field_t *board;            ///< tablica przechowująca wskaźniki na pola
} gamma_t;

/** @brief Zwraca wskaźnik na gracza o danym numerze.
 * @param[in] g       - wskaźnik na strukturę przechowującą stan gry,
 * @param[in] p       - numer gracza.
 */
player_t *get_player(gamma_t *g, uint32_t p);

/** @brief Zwraca wskaźnik na pole o danych współrzędnych.
 * @param[in] g       - wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x       - numer kolumny,
 * @param[in] y       - numer wiersza.
 */
field_t *get_field(gamma_t *g, uint32_t x, uint32_t y);

/** @brief Przyłącza pole gracza do sąsiednich obszarów.
 * Sprawdza czy gracze na polach sąsiednich do pola (@p x, @p y) są tacy
 * sami jak gracz na polu (@p x, @p y). Jeśli tak to łączy obszary w jedną
 * całość i aktualizuje liczbę obszarów zajętych przez gracza.
 * @param[in] g       - wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x       - numer kolumny,
 * @param[in] y       - numer wiersza.
 */
void merge_with_areas(gamma_t *g, uint32_t x, uint32_t y);

/** @brief Liczy ilość sąsiednich pól zajętych przez tego samego gracza.
 * Liczy ile sąsiednich pól pola (@p x, @p y) jest zajętych przez gracza @p p,
 * licząc od lewego sąsiada do dolnego sąsiada zgodnie z ruchem wskazówek
 * zegara, z pominięciem liczenia dla pierwszych @p start pól. Jeśli
 * @p start = 0 to pod uwagę brani są wszyscy sąsiedzi.
 * @param[in] g       - wskaźnik na strukturę przechowującą stan gry,
 * @param[in] x       - numer kolumny,
 * @param[in] y       - numer wiersza,
 * @param[in] p       - wskaźnik na gracza,
 * @param[in] start   - ilość pominiętych początkowych sąsiadów.
 */
uint8_t count_same_players(gamma_t *g, uint32_t x, uint32_t y, player_t *p,
                           uint8_t start);

/** @brief Dodaje ilość zawsze wolnych pól dla gracza stojącego na danym polu
* Oblicza ilość wolnych sąsiednich pól i sprawdza czy wokół nich nie ma pola
* zajętego przez tego samego gracza żeby nie liczyć wolnych sąsiednich pól dwa
* razy.
* @param[in] g       - wskaźnik na strukturę przechowującą stan gry,
* @param[in] x       - numer kolumny,
* @param[in] y       - numer wiersza.
*/
void add_always_free(gamma_t *g, uint32_t x, uint32_t y);

/** @brief Usuwa z planszy pionek gracza
* Usuwa z planszy pionek gracza na pozycji (@p x, @p y). Aktualizowana jest
* liczba zajętych przez gracza pól, obszarów oraz pozostałych parametrów,
* które mogą ulec zmianie przy usunięciu pionka.
* @param[in] g       - wskaźnik na strukturę przechowującą stan gry,
* @param[in] x       - numer kolumny,
* @param[in] y       - numer wiersza.
*/
void remove_field(gamma_t *g, uint32_t x, uint32_t y);

/** @brief Zwraca szerokość pola gry.
* Zwraca szerokość pola gry na podstawie podanej liczby graczy.
* @param[in] n       - liczba graczy.
*/
uint32_t get_field_size(uint32_t n);

/** @brief Sprawdza czy są spełnione warunki konieczne złotego ruchu.
 * Dla danego gracza sprawdzane są warunki konieczne dla możliwości
 * wykonania złotego ruchu. Warunki to: niewykorzystanie złotego ruchu
 * w poprzednich ruchach, obecność "pionków" na planszy, obecność innych
 * graczy w grze. Przydatne do szybkiego odrzucenia możliwości wykonania
 * złotego ruchu.
 * na planszy.
 * @param[in] g       - wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  - numer gracza,
 * @return Wartość @p true, jeśli warunki konieczne są spełnione,
 * a @p false w przeciwnym przypadku.
 */
bool gm_necessary(gamma_t *g, uint32_t player);

/** @brief Sprawdza czy możliwe jest wykonanie złotego ruchu na danym polu.
 * Symuluje wykonanie złotego ruchu na danym polu przez danego gracza.
 * @param[in] g       - wskaźnik na strukturę przechowującą stan gry,
 * @param[in] player  - numer gracza,
 * @param[in] x       - numer kolumny,
 * @param[in] y       - numer wiersza.
 * @return Wartość @p true, jeśli złoty ruch na danym polu jest możliwy,
 * a @p false w przeciwnym przypadku.
 */
bool gm_field_possible(gamma_t *g, uint32_t player, uint32_t x, uint32_t y);

#endif /* HELPERF_H*/
