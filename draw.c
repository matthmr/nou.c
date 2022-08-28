#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <unistd.h>

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
				(suit == HEARTS || suit == DIAMONDS)?
				REDCARD:
				(suit == CLUBS || suit == SPADES)?
				BLACKCARD:
				SPECIALCARD);
}

static inline char* _card_number (char* buf, Number num) {
	return _str_embed_draw (buf, (char*) numpm[num]);
}

static inline char* _card_suit (char* buf, Suit suit) {
	return _str_embed_draw (buf, (suit == SPECIAL)? " ": (char*) suitpm[suit]);
}

static char* _draw_card_cell (char* buf, Card* card) {
	buf = _str_embed_draw (buf, "[");
	buf = _card_color (buf, card->suit);
	buf = _card_number (buf, card->number);
	buf = _card_suit (buf, card->suit);
	buf = _str_embed_draw (buf, __RESET__ "] ");
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
static void draw_command_prompt (char* buf) {
	char* _buf = buf;

	_buf = _str_embed_draw (_buf, "(P0) ");

	(void) memset (_buf, 0, display.deckbs - (_buf - buf));
	(void) puts (buf);
}

static void draw_game_deck (char* buf, uint dcardn, uint pcardn, Card* pcard) {
	char* _buf = buf;

	_buf = _str_embed_draw (_buf, "D: ");
	_buf = _itoa_draw (_buf, dcardn);
	_buf = _str_embed_draw (_buf, "\nP: ");
	_buf = _itoa_draw (_buf, pcardn);
	_buf = _str_embed_draw (_buf, " ");
	_buf = _draw_card_cell (_buf, pcard);
	_buf = _str_embed_draw (_buf, "\n");

	(void) memset (_buf, 0, display.deckbs - (_buf - buf));
	(void) puts (buf);
}

static void draw_players_deck (char* buf, Player* p) {
	uint pcardi = p->cardi;
	uint* pcards = p->cards;

	char* _buf = buf;

	for (uint i = 0; i < pcardi; i++) {
		_buf = _itoa_draw (_buf, (i+1));
		_buf = _str_embed_draw (_buf, ":");
		// for some reason `index' doesn't work here and I'm too tired to know why
		_buf = _draw_card_cell (_buf,
					&(deckr.deck[pcards[i]/deckr.cards][pcards[i]%deckr.cards]));
	}

	_buf = _str_embed_draw (_buf, "\n");

	//(void) memset (_buf, 0, display.deckbs - (_buf - buf));
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
// -- draw stack bottom -- //

// static void draw_stack

void init_display (uint botn) {
	uint pcardn = player->cardn;

	display.prompt = calloc ((display.promptbs = PROMPTLEN + ERRMSGLEN + 2),
				  sizeof (char));
	display.deck = calloc ((display.deckbs = DECKENTRYLEN + 1),
			       sizeof (char));
	display.entry = calloc ((display.entrybs = PLAYERENTRYLEN * botn),
				sizeof(char));
	display.player = calloc ((display.playerbs = pcardn),
				 sizeof (PlayerCardCell));

	draw_command_prompt (display.prompt);
	draw_players_deck (display.player, &playerbuf[0]);
	draw_game_deck (display.deck,
			deckr.cards - (deckr.playing + deckr.played),
			deckr.played, top);
	draw_players_entry (display.entry, (playern+1), 0);
	exit (0); //DEBUG
}

void error_display (const char* msg) {

}

void update_display (Cmd* cmd) {

}
