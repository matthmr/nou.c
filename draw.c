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
 *     === `nou's drawing stack ===
 * 
 *  (P0) p1h
 *
 *                    (draw_command_prompt)
 *  1:[J ♣] 2:[1 ♥]
 *                    (draw_players_deck)
 *  D: 12
 *  P: 24 [3 ♣]
 *                    (draw_game_deck)
 *  P0: 2
 *  P1: 7
 *  P2: 2
 *  P3: 3 [*]
 *  P4: 8
 *  P5: 2
 *  P6: 10            (draw_players_entry)
 * 
 */

#define REDCARD __ESC__ "91m"
#define BLACKCARD __ESC__ "90m"
#define SPECIALCARD __ESC__ "1m"

Display display;
uint lines = 0;

static char movbackbuf[3 + MAXCARDSLEN + 4]; /* <esc>[<n>b<esc>[5c */
static uint movbackbuflen = 0;

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

static inline void allocbuf (Display* display) {
	display->buf = realloc (display->buf, display->bufs);
	display->buf[display->bufs - 1] = '\0';
}

static inline void render (char* buf, uint bufs) {
	write (1, buf, bufs);
}

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
												 __RESET__ "]\n" ERASE2ENDLINE:
												 __RESET__ "] " );

	return buf;
}

static char* _draw_players_entry (char* buf, uint i, Player* p) {
	buf = _str_embed_draw (buf, ERASE2ENDLINE "P");
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

static void movbackbuf_from_lines (uint lines) {
	char* _buf = movbackbuf;

	_buf = _str_embed_draw (_buf, __ESC__);
	_buf = _itoa_draw (_buf, lines);
	_buf = _str_embed_draw(_buf, "F");
	_buf = _str_embed_draw(_buf, MOVRIGHT ("5"));

	*_buf = '\0';

	movbackbuflen = (_buf - movbackbuf);
}

// -- draw stack top -- //
static char* draw_command_prompt (char* buf, bool is_player) {
	char* _buf = buf;

	_buf = _str_embed_draw (_buf,
													is_player?
													ERASE2ENDLINE "(P0) \n" ERASE2ENDLINE "\n":
													"\r" ERASE2ENDLINE "(P0) \n" ERASE2ENDLINE "\n");
	
	_buf = _str_embed_draw (_buf, "\n");

	return _buf;
}

static char* draw_players_deck (char* buf, Player* p) {
	static uint prevcardi = CPPLAYER;
	static uint prevdecklines = 1;

	uint pcardi = p->cardi;
	uint* pcards = p->cards;

	char* _buf = buf;

	// trigger line (re)counting
	if (prevcardi != pcardi) {
		prevcardi = pcardi;
		uint decklines = ((pcardi-1) / PLAYERDECKCOLUM) + 1;

		if (decklines < prevdecklines) {
			lines -= (prevdecklines - decklines);
			movbackbuf_from_lines (lines);
		}

		else if (decklines > prevdecklines) {
			lines += (decklines - prevdecklines);
			movbackbuf_from_lines (lines);
		}

		prevdecklines = decklines;
	}

	_buf = _str_embed_draw (_buf, ERASE2ENDLINE);

	for (uint i = 0; i < pcardi; i++) {
		_buf = _itoa_draw (_buf, (i+1));
		_buf = _str_embed_draw (_buf, ":");
		_buf = _draw_card_cell (_buf, &index (deckr.deck, pcards[i], deckr.cards), i);
	}

	// avoid a redundant newline; it can break the display
	if (pcardi % PLAYERDECKCOLUM) {
		_buf = _str_embed_draw (_buf, "\n" ERASE2ENDLINE);
	}

	_buf = _str_embed_draw (_buf, "\n");

	return _buf;
}

static char* draw_game_deck (char* buf, uint dcardn, uint pcardn, Card* pcard) {
	char* _buf = buf;
	Card _pcard = *pcard;

	_buf = _str_embed_draw (_buf, ERASE2ENDLINE "D: ");
	_buf = _itoa_draw (_buf, dcardn);
	_buf = _str_embed_draw (_buf, "\n" ERASE2ENDLINE "P: ");


	// avoid drawing if there are no cards played
	if (pcardn) {
		//_buf = _str_embed_draw (_buf, " ");

		// draw the chosen suit after inside the special card
		if (pcard->suit == SPECIAL) {
			_pcard.suit = csuit;
			pcard = &_pcard;
		}

		_buf = _draw_card_cell (_buf, pcard, 0);
	}

	_buf = _str_embed_draw (_buf, "\n" ERASE2ENDLINE);
	_buf = _str_embed_draw (_buf, "\n");

	return _buf;
}

static char* draw_players_entry (char* buf, uint playern, uint playert) {
	char* _buf = buf;

	// keep `P0' at the top
	if (playert == 0) {
		_buf = _draw_players_entry_highlight (_buf, 0, &playerbuf[0]);
		_buf = _str_embed_draw (_buf, __RESET__ "\n");
	}
	else {
		_buf = _draw_players_entry (_buf, 0, &playerbuf[0]);
		_buf = _str_embed_draw (_buf, "\n");
	}

	if (draw_players_entry_reverse)
		goto reverse_draw;

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

static void send_cursor_to_prompt (void) {
	write (1, movbackbuf, movbackbuflen);
}
// -- draw stack bottom -- //

static void draw_stack (uint pid, bool is_player) {
	char* _buf = display.buf;

	_buf = draw_command_prompt (_buf, is_player);
	_buf = draw_players_deck (_buf, player);
	_buf = draw_game_deck (_buf,
												 (deckr.cards - (deckr.playing + deckr.played)),
												 deckr.played, top);
	_buf = draw_players_entry (_buf, playern, pid);

	// NOTE: `_buf' is always one more than the drawer's head, so nulling
	// it in the end to truncates the render
	*_buf = '\0';

	render (display.buf, (_buf - display.buf));
	send_cursor_to_prompt ();
}

static void prompt_from_error (void) {
	// NOTE: this renders before the draw stack can render

	char buf[12];
	char* _buf = buf;

	_buf = _str_embed_draw (_buf, MOVUP0 ("2"));
	_buf = _str_embed_draw (_buf, MOVRIGHT ("5"));
	_buf = _str_embed_draw (_buf, ERASE2ENDLINE);

	write (1, buf, (_buf - buf));
}

void init_display (uint botn) {
	display.bufs = 0;
	display.buf = NULL;

	display.bufs += (PROMPTLEN + ERRMSGLEN); /* draw_command_prompt */
	display.bufs += CARDN * sizeof (PlayerCardCell); /* draw_players_entry */
	display.bufs += (DECKENTRYLEN); /* draw_game_deck */
	display.bufs += (botn-1) * sizeof (PlayerEntry) + sizeof (PlayerHighlightEntry); /* draw_players_entry */

	allocbuf (&display);

	// NOTE: `lines' include newlines :)
	lines = \
/* draw_command_prompt */ 3 + \
/* draw_players_deck */   ((player->cardi / PLAYERDECKCOLUM) + 1) + \
/* draw_game_deck */      3 + \
/* draw_players_entry */  botn + \
/* `puts' */              1;

	movbackbuf_from_lines (lines);

	draw_stack (0, true);
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

void update_display (Cmd* cmd, Player* p) {
	static uint _cardn = CARDN;

	Player* pplayer = cmd->p;
	uint pid = pplayer - player;

	// account for the newline: C-d handle is done at `cmdread'-time
	if (p->tag == PLAYER) {
		write (1, MSG (MOVUP0 ("1")));
	}

	// increase the buffer size if the player says so
	if (pplayer->cardn != _cardn) {
		_cardn = pplayer->cardn;
		display.bufs += (CARDN * sizeof (PlayerCardCell));
		allocbuf (&display);
	}

	draw_stack (pid, p->tag == PLAYER);
}

// TODO: fix fix_display
void fix_display (void) {
	write (1, MSG ("\r"));
	//draw_command_prompt (display.prompt);
}

#define HELPLINES 23

static void render_help (void) {
	char buf[HELPLINES*4*2];
	char* _buf = buf;

	for (uint i = 0; i < HELPLINES; i++) {
		_buf = _str_embed_draw (_buf, MOVDOWN0 ("1"));
		_buf = _str_embed_draw (_buf, ERASE2ENDLINE);
	}

	write (1, buf, (_buf - buf));
	_buf = buf;

	for (uint i = 0; i < HELPLINES; i++) {
		_buf = _str_embed_draw (_buf, MOVUP0 ("1"));
	}

	write (1, buf, (_buf - buf));
}

static void clear_help (Cmd* cmd) {
	char buf[HELPLINES*4*2];
	char* _buf = buf;

	uint pid = cmd->p - player;

	// clean up
	for (uint i = 0; i < HELPLINES; i++) {
		_buf = _str_embed_draw (_buf, MOVUP0 ("1"));
		_buf = _str_embed_draw (_buf, ERASE2ENDLINE);
	}

	//_buf = _str_embed_draw (_buf, MOVDOWN0 ("1"));

	write (1, buf, (_buf - buf));

	draw_stack (pid, true);
}

int draw_help_msg (Cmd* cmd) {
	// render the help message
	render_help ();

	char buf[2];
	buf[0] = buf[1] = 0;

	write (1, fullmsg, fullmsgsize);
	read (0, buf, sizeof (buf));

	// little-endian spiel
	short _buf = (buf[1] << 8) | buf[0];

	// TODO: rework this: ..
	// clear the message preemptively; if erroed, this function
	// will be called again
	clear_help (cmd);
	
	return (_buf != 0x000a);
}

void reverse_draw_players (void) {
	draw_players_entry_reverse ^= true;
}

void end_as_win (Player* playerwin) {
	char buf[1+MAXCARDSLEN+7] = {0};
	char* _buf = buf;

	_buf = _str_embed_draw (_buf, "P");
	_buf = _itoa_draw (_buf, playerwin - &playerbuf[0]);
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
