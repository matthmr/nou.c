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

     === `nou's drawing stack ===

  (P0) p1h                   }
                             }
                             } (draw_command_prompt)
  1:[J ♣] 2:[1 ♥]            }
                             } (draw_players_deck)
  D: 12                      }
  P: 24 [3 ♣]                }
                             } (draw_game_deck)
  P0: 2                      }
  P1: 7                      }
  P2: 2                      }
  P3: 3 [*]                  }
  P4: 8                      }
  P5: 2                      }
  P6: 10                     } (draw_players_entry)

*/

#define REDCARD __ESC__ "91m"
#define BLACKCARD __ESC__ "90m"
#define SPECIALCARD __ESC__ "1m"

Display display;
uint lines = 0;

bool block = false, reverse = false;

static char movbuf[3 + MAXCARDSLEN + 4];
static uint movbuflen = 0;

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
				(suit == CLUBS || suit == SPADES)? BLACKCARD:
				SPECIALCARD);
}

static inline char* _card_number (char* buf, Number num) {
	return _str_embed_draw (buf, (char*) numpm[num]);
}

static inline char* _card_suit (char* buf, Suit suit) {
	return _str_embed_draw (buf,
				(suit == SPECIAL)? " ": (char*) suitpm[suit]);
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

static void movbuf_from_lines (uint lines) {
	char* _buf = movbuf;

	_buf = _str_embed_draw (_buf, __ESC__);
	_buf = _itoa_draw (_buf, lines);
	_buf = _str_embed_draw(_buf, "F");
	_buf = _str_embed_draw(_buf, MOVRIGHT ("5"));

	*(_buf+1) = '\0';
	_buf += 1;

	movbuflen = (_buf - movbuf);
}

// -- draw stack top -- //
static void draw_command_prompt (char* buf) {
	char* _buf = buf;

	_buf = _str_embed_draw (_buf,
				ERASE2ENDLINE "(P0) \n" ERASE2ENDLINE "\n");

	(void) memset (_buf, 0, display.promptbs - (_buf - buf));
	(void) puts (buf);
}

static void draw_players_deck (char* buf, Player* p) {
	static uint prevcardi = CPPLAYER;
	static uint prevlines = 1;

	uint pcardi = p->cardi;
	uint* pcards = p->cards;

	char* _buf = buf;

	// trigger line (re)counting
	if (prevcardi != pcardi) {
		prevcardi = pcardi;
		uint _lines = (((pcardi-1) / PLAYERDECKCOLUM) + 1);

		if (_lines < prevlines) {
			lines--;
			movbuf_from_lines (lines);
		}

		else if (_lines > prevlines) {
			lines++;
			movbuf_from_lines (lines);
		}

		prevlines = _lines;
	}

	_buf = _str_embed_draw (_buf, ERASE2ENDLINE);

	for (uint i = 0; i < pcardi; i++) {
		_buf = _itoa_draw (_buf, (i+1));
		_buf = _str_embed_draw (_buf, ":");
		// for some reason `index' doesn't work here and I'm too tired to know why

		_buf = _draw_card_cell (_buf,
					&(deckr.deck[pcards[i]/deckr.cards][pcards[i]%deckr.cards]),
					i);
	}

	// avoid a redundant newline; it can break the display
	if (pcardi % PLAYERDECKCOLUM)
		_buf = _str_embed_draw (_buf, "\n" ERASE2ENDLINE);

	(void) memset (_buf, 0, display.playerbs - (_buf - buf));
	(void) puts (buf);
}

static void draw_game_deck (char* buf, uint dcardn, uint pcardn, Card* pcard) {
	char* _buf = buf;

	_buf = _str_embed_draw (_buf, ERASE2ENDLINE "D: ");
	_buf = _itoa_draw (_buf, dcardn);
	_buf = _str_embed_draw (_buf, "\n" ERASE2ENDLINE "P: ");
	_buf = _itoa_draw (_buf, pcardn);
	_buf = _str_embed_draw (_buf, " ");
	_buf = _draw_card_cell (_buf, pcard, 0);
	_buf = _str_embed_draw (_buf, "\n" ERASE2ENDLINE);

	(void) memset (_buf, 0, display.deckbs - (_buf - buf));
	(void) puts (buf);
}

static void draw_players_entry (char* buf, uint playern, uint playert) {
	char* _buf = buf;

	for (uint i = 0; i < playern; i++) {
		if (playert == i) {
			_buf = _draw_players_entry_highlight (_buf, i, &playerbuf[i]);
			_buf = _str_embed_draw (_buf, __RESET__ "\n");
			continue;
		}
		else {
			_buf = _draw_players_entry (_buf, i, &playerbuf[i]);
			_buf = _str_embed_draw (_buf, "\n");
			continue;
		}
	}

	(void) memset (_buf, 0, display.entrybs - (_buf - buf));
	(void) puts (buf);

}

static void send_prompt (void) {
	write (1, movbuf, movbuflen);
}

static void draw_stack (uint playert) {
	draw_command_prompt (display.prompt);
	draw_players_deck (display.player, player);
	draw_game_deck (display.deck,
			(deckr.cards - (deckr.playing + deckr.played)),
			deckr.played, top);
	draw_players_entry (display.entry, playern, playert);

	send_prompt ();
}	

static void prompt_from_error (void) {
	// this draws before the stack

	char buf[12];
	char* _buf = buf;

	_buf = _str_embed_draw (_buf, MOVUP0 ("2"));
	_buf = _str_embed_draw (_buf, MOVRIGHT ("5"));
	_buf = _str_embed_draw (_buf, ERASE2ENDLINE);

	write (1, buf, (_buf - buf));
}
// -- draw stack bottom -- //

void init_display (uint botn) {
	// the `+4' in the `calloc' call are for the `ERASE2ENDLINE'
	// escape sequence prepended to the `write' call

	display.promptbs = (PROMPTLEN + ERRMSGLEN);
	display.prompt = calloc (display.promptbs + 4, sizeof (char));

	display.playerbs = CARDN * sizeof (PlayerCardCell);
	display.player = calloc (display.playerbs + \
				 4 * (((player->cardi-1) / PLAYERDECKCOLUM) + 1),
				 sizeof (char));

	display.deckbs = (DECKENTRYLEN);
	display.deck = calloc (display.deckbs + 4*2, sizeof (char));
	
	display.entrybs = (botn-1) * sizeof (PlayerEntry) \
		          + sizeof (PlayerHighlightEntry);
	display.entry = calloc (display.entrybs + 4*botn, sizeof (char));

	lines = 3 + /* draw_command_prompt */ \
		(((player->cardi-1) / PLAYERDECKCOLUM) + 1) + /* draw_players_deck */ \
		3 + /* draw_game_deck */      \
		botn + 2; /* draw_players_entry */

	movbuf_from_lines (lines);

	draw_stack (0);
}

void error_display (const char* msg) {
	static bool _error = false;

	// clear the previous message
	if (_error) write (1, MSG (ERASE2ENDLINE));
	else _error = true;

	puts (msg);
	prompt_from_error ();
}

void info_display (const char* msg, uint infon) { }

void update_display (Player* pplayer) {
	// account for the newline...
	write (1, MSG (MOVUP0 ("1")));

	draw_stack (pplayer - player);
}

void fix_display (void) {
	write (1, MSG ("\r"));
	draw_command_prompt (display.prompt);
}

#define HELPLINES 23

static void _draw_help (void) {
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

	// clean up
	for (uint i = 0; i < HELPLINES; i++) {
		_buf = _str_embed_draw (_buf, MOVUP0 ("1"));
		_buf = _str_embed_draw (_buf, ERASE2ENDLINE);
	}

	//_buf = _str_embed_draw (_buf, MOVDOWN0 ("1"));

	write (1, buf, (_buf - buf));

	draw_stack (cmd->p - player);
}

int draw_help (Cmd* cmd) {
	// render the help message
	_draw_help ();

	char buf[2];
	buf[0] = buf[1] = 0;

	write (1, fullmsg, fullmsgsize);
	read (0, buf, sizeof (buf));

	// little-endian spiel
	short _buf = (buf[1] << 8) | buf[0];

	// clear the message preemptively; if erroed, this function
	// will be called again
	clear_help (cmd);
	
	return (_buf != 0x000a);
}

void set_draw_players_entry_reverse (void) {
	
}
