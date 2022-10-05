#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "draw.h"
#include "deck.h"
#include "nou.h"
#include "cli.h"
#include "cmd.h"

/**
 * 
 *          === `nou's drawing stack ===
 * 
 *s | (P0) p1h
 *
 *                      (draw_command_prompt)
 *    1:[J ♣] 2:[1 ♥]
 *                      (draw_players_deck)
 *    D: 12
 *    P: 24 [3 ♣]
 *                      (draw_game_deck)
 *    P0: 2
 *    P1: 7
 *    P2: 2
 *    P3: 3 [*]
 *    P4: 8
 *    P5: 2
 *    P6: 10            (draw_players_entry)
 *e |
 */

#define REDCARD __ESC__ "91m"
#define BLACKCARD __ESC__ "90m"
#define SPECIALCARD __ESC__ "1m"

#if DEBUG == 1
static Display debugdisplay;
#endif

Display display;
uint lines = 0;

static bool draw_players_entry_reverse = false;

const schar numpm[] = {
	[_A] = "A ",
	[_2] = "2 ",
	[_3] = "3 ",
	[_4] = "4 ",
	[_5] = "5 ",
	[_6] = "6 ",
	[_7] = "7 ",
	[_8] = "8 ",
	[_9] = "9 ",
	[_10] = "10 ",
	[_K] = "K ",
	[_Q] = "Q ",
	[_J] = "J ",
	[_B] = "B ",
	[_C] = "C ",
};

const wchar suitpm[] = {
	[DIAMONDS] = "♦",
	[CLUBS] = "♣",
	[HEARTS] = "♥",
	[SPADES] = "♠",
	[SPECIAL] = {0},
	// [HOLLOW (DIAMONDS)] = "♢",
	// [HOLLOW (CLUBS)] = "♧",
	// [HOLLOW (HEARTS)] = "♡",
	// [HOLLOW (SPADES)] = "♤",
};

#if DEBUG == 1
static inline void allocdebugbuf (Display* debugdisplay) {
	debugdisplay->buf = realloc (debugdisplay->buf, debugdisplay->bufs);
	debugdisplay->buf[debugdisplay->bufs - 1] = '\0';
}
#endif

static inline void allocbuf (Display* display) {
	display->buf = realloc (display->buf, display->bufs);
	display->buf[display->bufs - 1] = '\0';
}

static void render (Display display, char* _buf) {
	char* buf = display.buf;
	uint bufs = _buf - buf;

	// truncate the output
	*_buf = '\0';

	// render the draw buffer
	write (1, buf, bufs);
	
	// render the cursor
	write (1, MSG (MOVRIGHT ("5")));
}

#if DEBUG == 1
static void debugrender (char* buf, uint bufs) {
	write (2, buf, bufs);
}
#endif

static char* _itoa_draw (char* buf, uint i) {
	// find out how many digits there are
	uint _i = i;
	uint j = 0;
	do {
		_i /= 10;
		j++;
	} while (_i);

	// set up the limit
	uint exp = 1;
	for (uint _ = 1; _ < j; _++) exp *= 10;

	// convert top-bottom
	j = 0;
	while (exp) {
		uint div = (i / exp);
		buf[j] = ITOA (div);
		i = i - (exp * div);
		exp /= 10;
		j++;
	}
		
	buf += j;

	return buf;
}

static char* _str_embed_draw (char* buf1, char* buf2) {
	char c;
	uint i = 0;

	while ((c = buf2[i])) {
		buf1[i] = c;
		i++;
	}

	buf1 += i;

	return buf1;
}

static inline char* _card_color (char* buf, Suit suit) {
	return _str_embed_draw (buf,
													(suit == HEARTS || suit == DIAMONDS)? REDCARD:
													(suit == CLUBS  || suit == SPADES)?   BLACKCARD:
													SPECIALCARD);
}

static inline char* _card_number (char* buf, Number num) {
	return _str_embed_draw (buf, (char*) numpm[num]);
}

static inline char* _card_suit (char* buf, Suit suit) {
	return _str_embed_draw (buf, (suit == SPECIAL)? " ": (char*) suitpm[suit]);
}

static char* _draw_card_cell (char* buf, Card* card, uint i) {
	buf = _str_embed_draw (buf, "[");
	buf = _card_color (buf, card->suit);
	buf = _card_number (buf, card->number);
	buf = _card_suit (buf, card->suit);
	buf = _str_embed_draw (buf,
												 (((i+1) % PLAYERDECKCOLUM) == 0)?
												 __RESET__ "]\n":
												 __RESET__ "] " );

	return buf;
}

static char* _draw_players_entry (char* buf, uint i, Player* p) {
	buf = _str_embed_draw (buf, "P");
	buf = _itoa_draw (buf, i);
	buf = _str_embed_draw (buf, ": ");
	buf = _itoa_draw (buf, p->cardi);

	return buf;
}

static char* _draw_players_entry_highlight (char* buf, uint i, Player* p) {
	buf = _str_embed_draw (buf, __ESC__ __BOLD__);
	buf = _draw_players_entry (buf, i, p);
	buf = _str_embed_draw (buf, " [ * ]");

	return buf;
}

// -- draw stack top -- //
static char* draw_command_prompt (char* buf, bool is_player) {
	char* _buf = buf;

	// the player would've triggered the cursor to move back already
	_buf = _str_embed_draw (_buf,
													is_player?
													("(P0) \n\n\n"):
													("\r(P0) \n\n\n"));

	return _buf;
}

static char* draw_players_deck (char* buf, Player* p) {

	uint pcardi = p->cardi;
	uint* pcards = p->cards;

	char* _buf = buf;

	for (uint i = 0; i < pcardi; i++) {
		_buf = _itoa_draw (_buf, (i+1));
		_buf = _str_embed_draw (_buf, ":");
		_buf = _draw_card_cell (_buf,
														&index (deckr.deck, pcards[i], deckr.cards), i);
	}

	// avoid a redundant newline: if it ends on `PLAYERDECKCOLUM', it already
	// has a new line
	if (pcardi % PLAYERDECKCOLUM) {
		_buf = _str_embed_draw (_buf, "\n");
	}

	_buf = _str_embed_draw (_buf, "\n");

	return _buf;
}

#if DEBUG == 1
static void bot_draw (Player* bot) {
	char* _buf = debugdisplay.buf;

	_buf = draw_players_deck (_buf, bot);

	debugrender (debugdisplay.buf, (_buf - debugdisplay.buf));
}
#endif

static char* draw_game_deck (char* buf, uint dcardn, uint pcardn, Card* pcard) {
	char* _buf = buf;
	Card _pcard = *pcard;

	_buf = _str_embed_draw (_buf, "D: ");
	_buf = _itoa_draw (_buf, dcardn);
	_buf = _str_embed_draw (_buf, "\nP: ");
	_buf = _itoa_draw (_buf, pcardn);
	_buf = _str_embed_draw (_buf, " ");

	// avoid drawing if there are no cards played
	if (pcardn) {
		// draw the chosen suit after inside the special card
		if (pcard->suit == SPECIAL) {
			_pcard.suit = csuit;
			pcard = &_pcard;
		}
		_buf = _draw_card_cell (_buf, pcard, 0);
	}

	_buf = _str_embed_draw (_buf, "\n\n");

	return _buf;
}

static char* draw_players_entry (char* buf, uint playern, uint playert) {
	char* _buf = buf;

	// keep `P0' at the top
	if (playert == 0) {
		_buf = _draw_players_entry_highlight (_buf, 0, player);
		_buf = _str_embed_draw (_buf, __RESET__ "\n");
	}
	else {
		_buf = _draw_players_entry (_buf, 0, &playerbuf[0]);
		_buf = _str_embed_draw (_buf, "\n");
	}

	if (draw_players_entry_reverse) {
		goto reverse_draw;
	}

	for (uint i = 1; i < playern; i++) {
		if (playert == i) {
			_buf = _draw_players_entry_highlight (_buf, i, &playerbuf[i]);
			_buf = _str_embed_draw (_buf, __RESET__ "\n");
		}
		else {
			_buf = _draw_players_entry (_buf, i, &playerbuf[i]);
			_buf = _str_embed_draw (_buf, "\n");
		}
	}
	goto done;

reverse_draw:
	for (uint i = (playern-1); i > 0; i--) {
		if (playert == i) {
			_buf = _draw_players_entry_highlight (_buf, i, &playerbuf[i]);
			_buf = _str_embed_draw (_buf, __RESET__ "\n");
		}
		else {
			_buf = _draw_players_entry (_buf, i, &playerbuf[i]);
			_buf = _str_embed_draw (_buf, "\n");
		}
	}

done:
	_buf = _str_embed_draw (_buf, "\n");

	return _buf;
}

static uint recomp_lines (uint lines, bool is_player) {
	static uint prevcardi = CPPLAYER;
	static uint prevdecklines = 1;

	uint pcardi = player->cardi;

	if (! is_player) {
		goto done;
	}

	if (prevcardi != pcardi) {
		prevcardi = pcardi;
		uint decklines = ((pcardi-1) / PLAYERDECKCOLUM) + 1;

		if (decklines < prevdecklines) {
			lines -= (prevdecklines - decklines);
		}
		else if (decklines > prevdecklines) {
			lines += (decklines - prevdecklines);
		}

		prevdecklines = decklines;
	}

done:
	return lines;
}

static char* draw_cursor (char* buf, uint lines) {
	char linebuf[MAXCARDSLEN + 1];
	char* _linebuf = linebuf;

	buf = _str_embed_draw (buf, "\x1b[");
  _linebuf = _itoa_draw (_linebuf, lines);
	*_linebuf = '\0';
	buf = _str_embed_draw (buf, linebuf);
	buf = _str_embed_draw (buf, "F");

	return buf;
}

static char* draw_clear (char* buf, uint prevlines) {
	uint bound =
		(prevlines < lines)? lines:
		(prevlines > lines)? prevlines + (prevlines - lines):
		prevlines;

	for (uint i = 0; i < bound; i++) {
		buf = _str_embed_draw (buf, ERASE2ENDLINE "\n");
	}

	// send the draw cursor back
	buf = draw_cursor (buf, bound);

	return buf;
}
// -- draw stack bottom -- //

static void draw_stack (uint pid, bool is_player) {
	char* _buf = display.buf;
	uint prevlines = lines;

	uint deckcards = deckr.cards - (deckr.playing + deckr.played);

	lines = recomp_lines (lines, is_player);

	// clear the previous screen
	_buf = draw_clear (_buf, prevlines);

	// -- draw stack top -- //
	_buf = draw_command_prompt (_buf, is_player);
	_buf = draw_players_deck (_buf, player);
	_buf = draw_game_deck (_buf, deckcards, deckr.played, top);
	_buf = draw_players_entry (_buf, playern, pid);
	// -- draw stack bottom -- //

	// put the cursor back
	_buf = draw_cursor (_buf, lines);

	// render it out
	render (display, _buf);
}

// NOTE: this renders before the draw stack can render
static void prompt_from_error (void) {
	char buf[13];
	char* _buf = buf;

	// move back to the prompt
	_buf = _str_embed_draw (_buf, MOVUP0 ("2"));
	_buf = _str_embed_draw (_buf, MOVRIGHT ("5"));

	// erase the previous command
	_buf = _str_embed_draw (_buf, ERASE2ENDLINE);

	*_buf = '\0';

	write (1, buf, sizeof (buf));
}

void init_display (uint botn) {
	display.bufs = 0;
	display.buf = NULL;

	display.bufs += (PROMPTLEN + ERRMSGLEN); /* draw_command_prompt */
	display.bufs += CARDN * sizeof (PlayerCardCell); /* draw_players_deck */
	display.bufs += (DECKENTRYLEN); /* draw_game_deck */
	display.bufs += (botn-1) * sizeof (PlayerEntry) + sizeof (PlayerHighlightEntry); /* draw_players_entry */

	lines = \
/* `draw_command_prompt' */ 3 + \
/* `draw_players_deck' */   ((player->cardi / PLAYERDECKCOLUM) + 1) + \
/* `draw_game_deck' */      3 + \
/* `draw_players_entry' */  botn + \
/* `puts' */                1;

	// `draw_clear' buffer going down
	display.bufs += (lines - \
		((player->cardi / PLAYERDECKCOLUM) + 1) + \
		(((DECKS (botn) * CPDECK) / PLAYERDECKCOLUM) + 1)) * 5;

	// `draw_clear' buffer going up
	display.bufs += 20;

	// the last null byte
	display.bufs++;

	allocbuf (&display);

	draw_stack (0, true);

	#if DEBUG == 1
	debugdisplay.bufs += CARDN * sizeof (PlayerCardCell);
	allocdebugbuf (&debugdisplay);
	allocdebugbuf (&debugdisplay);
	bot_draw (&playerbuf[1]);
	#endif
}

void error_display (const char* msg) {
	static bool _error = false;

	// clear the previous message
	if (_error) {
		write (1, MSG (ERASE2ENDLINE));
	}
	else {
		_error = true;
	}

	puts (msg);
	prompt_from_error ();
}

void info_display (const char* msg, uint infon) { }

void update_display (Cmd* cmd, Player* p, bool statslave) {
	static uint _cardn = CARDN;

	Player* pplayer = cmd->p;
	Ptag ptag = p->tag;
	uint pid = pplayer - player;

	// account for the newline: C-d handle is done at `cmdread'-time
	if (ptag == PLAYER && !statslave) {
		write (1, MSG (MOVUP0 ("1")));
	}

	#if DEBUG == 1
	else {
		if (pplayer->cardn != _cardn) {
			_cardn = cmd->p->cardn;
			debugdisplay.bufs += (CARDN * sizeof (PlayerCardCell));
			allocdebugbuf (&debugdisplay);
		}
		bot_draw (&playerbuf[1]);
	}
	#endif

	// increase the buffer size if the player says so
	if (pplayer->cardn != _cardn) {
		_cardn = pplayer->cardn;
		display.bufs += (CARDN * sizeof (PlayerCardCell));
		allocbuf (&display);
	}

	draw_stack (pid, ptag == PLAYER);
}

// TODO: fix fix_display
void fix_display (void) {
	write (1, MSG ("\r"));
	//draw_command_prompt (display.prompt);
}

void reverse_draw_players (void) {
	draw_players_entry_reverse ^= true;
}

void end_as_win (Player* playerwin) {
	char buf[1+MAXCARDSLEN+7] = {0};
	char* _buf = buf;

	uint win = (playerwin - player);

	_buf = _str_embed_draw (_buf, "P");
	_buf = _itoa_draw (_buf, win);
	_buf = _str_embed_draw (_buf, " WINS!");

	write (1, MSG (ERASE2ENDLINE));

	// P<n> WINS!
	puts (buf);
}

void end_as_draw (void) {
	write (1, MSG (ERASE2ENDLINE));

	// DRAW! 
	puts ("DRAW!");
}
