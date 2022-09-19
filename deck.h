#ifndef LOCK_DECK
#  define LOCK_DECK

#  include "utils.h"

#  define index(d,x,y) (d)[(x)/(y)][(x)%(y)]
#  define CARDN 20

typedef enum direction {
	DIRCLOCKWISE = 0,
	DIRCOUNTERCLOCKWISE = -1u,
} Direction;
#  define DIRECTION DIRCLOCKWISE

typedef enum suit {
	DIAMONDS = BIT (1), // 1
	CLUBS = BIT (2),    // 2
	HEARTS = BIT (3),   // 4
	SPADES = BIT (4),   // 8
	SPECIAL = BIT (5),  // 16

	NOSUIT = 0,
} Suit;

typedef enum number {
	_A = 0, _2, _3, _4, _5, _6, _7, _8, _9, _10,
	_K, // block    (10)
	_Q, // reverse  (11)
	_J, // +2			  (12)
	_B, // +4			  (13)
	_C, // joker	  (14)

	NONUMBER = -1,
} Number;

#  include "players.h"

typedef struct card {
	Suit suit;
	Number number;
	Player* owner;
} Card;

typedef Card Deck[CPDECK];

typedef struct deckr {
	Deck* deck;
	uint n, cards;
	uint playing, played;
} Deckr;

extern Deckr deckr;

extern uint seed, reseed;
extern Card* top;

extern Card* stacktop, * stackbase;
extern Card* decktop, * deckbase;

extern Suit csuit;
extern uint acc, pacc;

extern Direction dir;

uint reseedr (uint);
uint seedr (uint);
uint seeded (uint);

void shuffle (Deckr*, uint, uint);

void popplayers (Deckr*, uint, uint);
void popdeck (Deckr*);

#  define NOCARDS 1
int sort (void);

#endif
