#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#include "nou.h"
#include "cli.h"

Deck* deck = NULL;
Card* top = NULL;
Direction dir = DIRECTION;

static int playing = true;

static void sort (Deckr* deckr) { // sort the deck to make mount cards stack-like
}

static void popdeck (Deckr* deckr) { // populate the playing deck
	Card card;
	Deck* deck = deckr->deck;
	uint n = deckr->n;
	uint i = 0, c = 0;
	card.playing = 0;
	iter: for (Suit suit = DIAMONDS; suit <= SPADES; suit++) {
		for (Number number = _A; number <= _10; number++) {
			card.suit = suit;
			card.number = number;
			card.playing = card.played = 0;
			deck[i][c] = card;
			c++;
		}
	}
	c++;
	card.suit = SPECIAL;
	card.number = _B;
	card.playing = card.played = 0;
	deck[i][c] = card;
	c++;
	card.suit = SPECIAL;
	card.number = _C;
	card.playing = card.played = 0;
	deck[i][c] = card;
	c = 0;
	i++;
	if (i != n) goto iter;
};

static void swap (Deckr* deckr, uint r1l, uint r1u, uint r2l, uint r2u) {}

static void propshuf (Deckr* deckr, uint partn, uint mask) { // backpropagate the suffling
	uint n = deckr->n;
	Deck* deck = deckr->deck;
	uint _n = n * CPDECK;

	uint upper = _n, lower = 0;
	uint _mask = 1, __mask = 0;
	uint intval = upper - lower;

	Prop p[partn];
	Card* card = NULL;

	for (uint _i = 1; _i <= partn; _i++) { // populate `p'
		__mask = mask & _mask;
		_mask <<= _i;
		if (__mask) { // partition left (1)
			upper -= intval / 2;
			intval = upper - lower;
			if (upper > CPDECK) {
				uint i = upper % CPDECK;
				card = &deck[i][upper - (i*CPDECK)];
			}
			else card = &deck[0][upper];
		}
		else { // partition right (0)
			lower += intval / 2;
			intval = upper - lower;
			if (lower > CPDECK) {
				uint i = lower % CPDECK;
				card = &deck[i][(lower - (i*CPDECK)) + 1];
			}
			else card = &deck[0][lower];
		}
		Prop prop = {card, __mask};
		p[_i - 1] = prop;
	}

}

static inline uint cmask (uint r, uint bits, uint off) { // mask the random bits
	r >>= off;
	uint mask = ~((~0UL) << bits);
	return r & mask;
}

static void shuffle (Deckr* deckr, uint pcards) { // shuffle the playing deck
	uint r = time (NULL);
	uint bits = pcards, partn = 0;
	do {
		bits >>= 1;
		partn++;
	} while (bits); // get the ammount of choices
	if (partn > 1) partn--;
	uint bitoff = (r % ((sizeof (uint)*8) - partn)); // get a random offset in `r'
	r = (uint) random ();
	uint mask = cmask (r, partn, bitoff); // get the bits of `r' in that offset
	propshuf (deckr, partn, mask);
}

static void popbots (Deckr* deckr, uint botn) { }
static void popplayer (Deckr* deckr) { }

static void gameinit (Deckr* deckr, uint botn) { // initiate the game
	uint n = DECKS (botn+1);
	uint cards = n * CPDECK;
	deckr->n = n;
	deckr->cards = cards;
	deckr->deck = malloc (cards * sizeof (Card));
	deckr->playing = PLAYING (botn+1);
	deckr->played = 0;
	popdeck (deckr);
	shuffle (deckr, deckr->cards);
	popbots (deckr, botn);
	popplayer (deckr);
	sort (deckr);
	write (1, MSG (GAME_HEADER));
}

static int gameloop (uint botn) { // main game loop
	Deckr deckr = {
		.deck = deck,
		.n = 0,
		.cards = 0,
		.playing = 0,
		.played = 0
	};
	gameinit (&deckr, botn);

	while (playing) {
		//draw (gstate);
		goto end; // debug
	}

	end: return 0;
}

int main (int argc, char** argv) {
	int bots = BOTS;
	if (argc > 1) {
		if ((argv[1] && argv[1][0] == '-')) {
			switch (argv[1][1]) {
			case 'v': write (1,
				MSG ("nou " VERSION "\n")); return 1;
			case 'h': default: write (2,
				MSG ("Usage: nou [bot number:" BOTSSTR "]\n"
			            "Note:  type `q' and press enter to quit at moment\n")); return 1;
			}
		}
		bots = atoi (argv[1]);
		if (bots <= 1)
			write (2, MSG ("[ WW ] Invalid number, defaulting to " BOTSSTR "\n"));
		else if (bots >= SOFTLIM)
			write (2, MSG ("[ WW ] Too many bots, defaulting to " BOTSSTR "\n"));
		else goto loop;
		bots = BOTS;
	}
	loop: return gameloop (bots);
}
