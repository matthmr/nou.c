#ifndef LOCK_DRAW
#  define LOCK_DRAW

#  include "bots.h"
#  include "utils.h"

typedef char wchar[4];
typedef char schar[4/*3*/];

#  define PROMPTLEN (6) // (P0) 
#  define CARDLEN (1 + sizeof (schar) + 1 + sizeof (wchar) + 1) // [J ♣]
#  define ESCLEN (8) // <esc>[..;..m
#  define MAXCARDSLEN (5) // got `18036`
#  define PLAYERENTRYLEN (ESCLEN + 7 + MAXCARDSLEN + 3 + ESCLEN \
                       + 1) /* <esc>P<n>: <m> [*]<esc> */
#  define DECKENTRYLEN (3 + MAXCARDSLEN + 1         /* D: <n>       */ \
                        + 3 + MAXCARDSLEN + CARDLEN /* P: <n> [J ♣] */ \
                        + 1)

#  define HOLLOW(x) ((x) + (SPADES+1))

// #  define STR(x) x "\0" // i forgot about C-style strings :p

typedef char ansi[ESCLEN];
typedef char CardNID[CARDLEN];

// [J ♣]
typedef char CardCell[1 \
		      + sizeof(ansi) + sizeof (schar) + 1 \
		      + sizeof (wchar) + sizeof (ansi) + 1];

// 1:[J ♣]
typedef char PlayerCardCell[sizeof (CardNID) + 1 + sizeof (CardCell)];

// P<n>: <m>
typedef char PlayerEntry[PLAYERENTRYLEN];

extern const schar numpm[];
extern const wchar suitpm[];

typedef struct display {
	char* player, * entry, * deck, * prompt;
	uint playerbs, entrybs, deckbs, promptbs;
} Display;

extern Display display;

void error_display (const char* msg);
void update_display (Cmd*);
void init_display (uint);

#endif

#ifndef LOCK_CLI_GAME_HEADER
#  define LOCK_CLI_GAME_HEADER

#  define GAME_HEADER "\n			|" BOLD ("nou " VERSION) "|\n" \
                      "\n => Made by mH (" STYLE ("37;4m", "https://github.com/matthmr") ")" \
                      "\n" //\n"

#  define PROMPT "(P0) "

#endif
