#ifndef LOCK_DRAW
#  define LOCK_DRAW

#  include "bots.h"
#  include "utils.h"

typedef char wchar[4];
typedef char schar[4];

#  define PLAYERDECKCOLUM (7)

#  define PROMPTLEN (8) // (P0) \n\n
#  define CARDLEN (1 \
		   + sizeof (ansi) + sizeof (schar) + 1 + sizeof (wchar) \
		   + sizeof (ansi) + 1) // [J ♣]
#  define ESCLEN (8) // <esc>[..;..m
#  define MAXCARDSLEN (5) // got `18036`
#  define MAXPLAYERLEN (3)

#  define PLAYERENTRYLEN (1 + MAXPLAYERLEN + 1 + 1 + MAXCARDSLEN + 1)      /* P<n>: <m> */
#  define PLAYERHIGHENTRYLEN (ESCLEN \
			      + 1 + MAXPLAYERLEN + 1 + 1 + \
			      MAXCARDSLEN + 1 + 3 + ESCLEN + 1)           /* <esc>P<n>: <m> [*]<esc> */
#  define DECKENTRYLEN (1 + 1 + 1 + MAXCARDSLEN + 1                       /* D: <n>       */ \
                        + 1 + 1 + 1 + MAXCARDSLEN + 1 + CARDLEN + 1)      /* P: <n> [J ♣] */

typedef char ansi[ESCLEN];
typedef char CardNID[MAXCARDSLEN];

// [J ♣]
typedef char CardCell[CARDLEN];

// 1:[J ♣]
typedef char PlayerCardCell[sizeof (CardNID) + 1 + sizeof (CardCell) + 1];

// P<n>: <m>
typedef char PlayerHighlightEntry[PLAYERHIGHENTRYLEN];
typedef char PlayerEntry[PLAYERENTRYLEN];

extern const schar numpm[];
extern const wchar suitpm[];

typedef struct display {
	char* player, * entry, * deck, * prompt;
	uint playerbs, entrybs, deckbs, promptbs;
} Display;

extern Display display;

void error_display (const char*);
void info_display (const char*, uint);
void update_display (Player*);
void init_display (uint);

#endif

#ifndef LOCK_CLI_GAME_HEADER
#  define LOCK_CLI_GAME_HEADER

#  define GAME_HEADER "\n			|" BOLD ("nou " VERSION) "|\n" \
                      "\n => Made by mH (" STYLE ("37;4m", "https://github.com/matthmr") ")" \
                      "\n" //\n"

#  define PROMPT "(P0) "

#endif
