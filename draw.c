#include "nou.h"
#include "bots.h"
#include "draw.h"

Display display;

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

void update_display (void) {

}
