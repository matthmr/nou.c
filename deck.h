#ifndef LOCK_DECK
#  define LOCK_DECK

#  include "utils.h"

#  define index(d,x,y) (d)[(x)/(y)][(x)%(y)]
#  define CARDN 20

typedef enum direction {
	DIRCLOCKWISE = 1,
	DIRCOUNTERCLOCKWISE = -1,
} Direction;
#  define DIRECTION DIRCLOCKWISE

typedef enum suit {
	SPECIAL = -1,
	DIAMONDS = 0,
	CLUBS,
	HEARTS,
	SPADES,
} Suit;

typedef enum number {
	_A = 0, _2, _3, _4, _5, _6, _7, _8, _9, _10,
	_K, // block
	_Q, // reverse
	_J, // +2
	_B, // +4
	_C, // joker
} Number;

typedef struct card {
	Suit suit;
	Number number;
	bool playing;
	// short played;
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
extern Card* card0;
extern Card* stacktop, * decktop;

extern Suit csuit;

extern Direction dir;

uint reseedr (uint);
uint seedr (uint);
uint seeded (uint);

void swap (Deck*, uint, uint);
void shuffle (Deckr*, uint);
void reshuffle (Deckr*, uint);

void popplayers (Deckr*, uint, uint);

#define NOCARDS 1
int sort (Deckr*);
void popdeck (Deckr*);

#endif
