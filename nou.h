#ifndef LOCK_NOU
#  define LOCK_NOU

#  include "utils.h"

#  define VERSION "v0.1.1"
#  define PROG "nou"

typedef char wchar[4];
typedef char schar[2];

typedef enum suit {
	SPECIAL = -1,
	DIAMONDS = 0,
	CLUBS,
	HEARTS,
	SPADES,
} Suit;

#  define HOLLOW(x) ((x) + (SPADES+1))

typedef enum number {
	_A, _2, _3, _4, _5, _6, _7, _8, _9, _10,
	_K, // block
	_Q, // reverse
	_J, // +2
	_B, // +4
	_C, // joker
} Number;

const schar numpm[] = {
	[_A] = "A",
	[_2] = "2",
	[_3] = "3",
	[_4] = "4",
	[_5] = "5",
	[_6] = "6",
	[_7] = "7",
	[_8] = "8",
	[_9] = "9",
	[_10] = "10",
	[_K] = "K",
	[_Q] = "Q",
	[_J] = "J",
	[_B] = "B",
	[_C] = "C",
};

const wchar suitpm[] = {
	[DIAMONDS] = "♦",
	[CLUBS] = "♣",
	[HEARTS] = "♥",
	[SPADES] = "♠",

	[HOLLOW (DIAMONDS)] = "♢",
	[HOLLOW (CLUBS)] = "♧",
	[HOLLOW (HEARTS)] = "♡",
	[HOLLOW (SPADES)] = "♤",
};


typedef struct card {
	Suit suit;
	Number number;
	short playing;
	short played;
} Card;

typedef Card Deck[CPDECK];

typedef struct draw {
	char _lpad;
	wchar suit;
	schar number;
} CardDraw;

typedef int Direction;

typedef struct prop {
	Card* card;
	Direction dir;
} Prop;

typedef struct player {
	uint* owns;
} Player;

typedef struct bot {
	Player p;
} Bot;

typedef struct deckr {
	Deck* deck;
	uint n;
	uint cards;
	uint playing;
	uint played;
} Deckr;

extern Deck* deck;
extern Card* last_played;
extern Direction dir;

#  define CW 0
#  define CCW 1
#  define DIRECTION CW
#  define REVERSE(x) ((x)+1)%2

#endif
