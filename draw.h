#ifndef LOCK_DRAW
#  define LOCK_DRAW

#  include "utils.h"
#  include "players.h"

typedef char wchar[4];
typedef char schar[4];

#  define PLAYERDECKCOLUM (7)

#  define PROMPTLEN (9)                                                    /* <esc>(P0) \n<msg> */
#  define CARDLEN (1 +                                                     /* [J ♣] */ \
									 sizeof (ansi) + sizeof (schar) + 1 + sizeof (wchar) + \
									 sizeof (ansi) + 1)
#  define ESCLEN (8)                                                       /* <esc>[..;..m */
#  define MAXPLAYERS (999)                                                 /* got `999' */
#  define MAXCARDS (18036)                                                 /* got `18036' */
#  define MAXCARDSLEN (5)                                                  /* got `18036' */
#  define MAXPLAYERLEN (3)                                                 /* got `999' */

#  define PLAYERENTRYLEN (1 + MAXPLAYERLEN + 1 + 1 + MAXCARDSLEN + 1)      /* P<n>: <m> */
#  define PLAYERHIGHENTRYLEN (ESCLEN +                                     /* <esc>P<n>: <m> [*]<esc> */ \
															1 + MAXPLAYERLEN + 1 + 1 + \
															MAXCARDSLEN + 1 + 3 + ESCLEN + 1)
#  define DECKENTRYLEN (1 + 1 + 1 + MAXCARDSLEN + 1 +                      /* D: <n>       */ \
                        1 + 1 + 1 + MAXCARDSLEN + 1 + CARDLEN + 1)         /* P: <n> [J ♣] */
typedef char ansi[ESCLEN];

// <n>:
typedef char CardNumId[MAXCARDSLEN];

// [J ♣]
typedef char CardCell[CARDLEN];

// 1:[J ♣]
typedef char PlayerCardCell[sizeof (CardNumId) + sizeof (CardCell) + 1];

// P<n>: <m>
typedef char PlayerHighlightEntry[PLAYERHIGHENTRYLEN];
typedef char PlayerEntry[PLAYERENTRYLEN];

extern const schar numpm[];
extern const wchar suitpm[];

typedef struct display {
	char* buf;
	uint bufs;
} Display;

extern Display display;

void error_display (const char*);
void info_display (const char*, uint);
void init_display (uint);
void fix_display (void);
void update_display (Cmd*, Player*, bool);

int draw_help_msg (Cmd*);

void end_as_win (Player*);
void end_as_draw (void);

void reverse_draw_players (void);

#endif

#ifndef LOCK_CLI_GAME_HEADER
#  define LOCK_CLI_GAME_HEADER

#  define GAME_HEADER "\n			|" BOLD ("nou " VERSION) "|\n" \
                      "\n => Made by mH (" STYLE ("37;4m", "https://github.com/matthmr") ")" \
                      "\n" //\n"

#  define PROMPT "(P0) "

#endif
