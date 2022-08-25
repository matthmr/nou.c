#ifndef LOCK_DRAW
#  define LOCK_DRAW

#  include "bots.h"
#  include "utils.h"

typedef char wchar[4];
typedef char schar[2];

#  define PROMPTLEN (6) // (P0) 
#  define CARDLEN (1 + sizeof (schar) + 1 + sizeof (wchar) + 1) // [J ♣]
#  define ESCLEN (8) // <esc>[..;..m
#  define MAXCARDSLEN (5) // got `18036`
#  define BOTENTRYLEN (ESCLEN + 7 + MAXCARDSLEN + 3 + ESCLEN /* <esc>P<n>: <m> [*]<esc> */ \
                       + 1)
#  define DECKENTRYLEN (3 + MAXCARDSLEN + 1 +     /* D: <n> */       \
                        3 + MAXCARDSLEN + CARDLEN /* P: <n> [J ♣] */ \
                        + 1)

#  define HOLLOW(x) ((x) + (SPADES+1))

typedef char ansi[ESCLEN];
typedef char CardNID[CARDLEN];

typedef struct {
	ansi* color;
	schar* number;
	wchar* suit;
} CardDraw; // 1:[J ♣]

typedef char CardCell[1 + sizeof(ansi) + sizeof (schar) + 1 + sizeof (wchar) + sizeof (ansi) + 1]; // [J ♣]
typedef char PlayerCardCell[sizeof (CardNID) + 1 + sizeof (CardCell)]; // 1:[J ♣]

extern const schar numpm[];
extern const wchar suitpm[];

typedef struct display_player {
	CardDraw* cards;
} DisplayPlayer;

typedef struct display {
	char* player, * bots, * deck, * prompt;
} Display;

extern CardCell ccell;
extern PlayerCardCell pcell;

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
