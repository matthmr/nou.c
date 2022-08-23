#include <stdlib.h>

#include "nou.h"
#include "deck.h"
#include "bots.h"

Direction dir = DIRECTION;

Deckr deckr = {
	.deck = NULL,
	.n = 0,
	.cards = 0,
	.playing = 0,
	.played = 0
};

//Deck* deck = NULL;
Card* top = NULL;
Card* card0 = NULL;
Card* stacktop = NULL, * decktop = NULL;

Suit csuit;

uint seed, reseed;

static Player** playerringbuf;

int sort (Deckr* deckr) { // sort the deck to make mount cards stack-like;
                          // cards that are being played stay at low memory, otherwise high memory

	// TODO: short circuit

	Deck* deck = deckr->deck;
	uint cardn = deckr->cards;
	uint playing = deckr->playing;

	Card* splaying = NULL;

	uint i = 0, _i = 0;

	for (; i < cardn; i++) {
		if (! (index (deck, i, cardn).playing)) {
			if (!splaying) {
				_i = i;
				splaying = &index (deck, i, cardn);
			}
		}
		else if (splaying) {
			playing--;
			splaying = NULL;
			Card tmp = index (deck, i, cardn);
			index (deck, i, cardn) = index (deck, _i, cardn);
			index (deck, _i, cardn) = tmp;
			i = _i;
		}
		if (!playing) break;
	}

	return splaying? NOCARDS: 0; // if there are no playable cards left
}

void popdeck (Deckr* deckr) { // populate the playing deck
	Card card;
	Deck* deck = deckr->deck;
	uint n = deckr->n;
	uint i = 0, c = 0;
	card.playing = 0;
	iter:
	for (Suit suit = DIAMONDS; suit <= SPADES; suit++)
		for (Number number = _A; number <= _J; number++) {
			card.suit = suit;
			card.number = number;
			card.playing = 0;
			deck[i][c] = card;
			c++;
		}
	//c++;
	card.suit = SPECIAL;
	card.number = _B;
	card.playing = 0;
	deck[i][c] = card;
	c++;
	card.suit = SPECIAL;
	card.number = _C;
	card.playing = 0;
	deck[i][c] = card;
	c = 0;
	i++;
	if (i != n) goto iter;
	card0 = &deckr->deck[0][0];
};

uint reseedr (uint primr) { // reseed our `seed' and `reseed' variables
	if (! primr) primr = seed + reseed;
	uint div = primr < seed? (seed / primr): (primr / seed);
	uint reseedbits = div % (sizeof reseed);
	reseed <<= reseedbits;
	reseed |= ONES (reseedbits);
	return seed ^ reseed;
}

uint seedr (uint primr) { // seed our `seed' and `reseed' variables
	if (! primr) primr = seed + reseed;
	uint div = primr < seed? (seed / primr): (primr / seed);
	uint reseedbits = div % (sizeof reseed);
	reseed |= seed;
	reseed <<= reseedbits;
	reseed ^= seed;
	return reseedr (reseed+1);
}

uint seeded (uint n) { // get `n' values from the `seed' variable
	uint ret = seed % n;
	seed = seedr (ret);
	return ret;
}

void swap (Deck* deck, uint i, uint pcards) { // swap cards
	//uint _i = seeded (pcards) + i;
	uint _i = (seeded (pcards) + i) % pcards;
	if (! _i) return;
	Card tmp = index (deck, i, pcards);
	index (deck, i, pcards) = index (deck, _i, pcards);
	index (deck, _i, pcards) = tmp;
}

void shuffle (Deckr* deckr, uint pcards) { // shuffle the playing deck
	Deck* deck = deckr->deck;
	for (uint i = 0; i < pcards; i++)
		swap (deck, i, pcards);
}

static void ringswap (Player** ring, uint i, uint botn) { // swap the `playerbufring'
	//uint _i = seeded (botn) + i;
	uint _i = (seeded (botn) + i) % botn;
	if (! _i) return;
	Player* tmp = ring[i];
	ring[i] = ring[_i];
	ring[_i] = tmp;
}

static inline void ringshuffle (Player** ring, uint botn) { // shuffle the `playerbufring'
	for (uint i = 0; i < botn; i++)
		ringswap (ring, i, botn);
}

static void popbots (uint botn) { // populate bots
	Player* bot;

	player = &playerbuf[0];
	player->tag = PLAYER;
	player->bot = NULL;
	player->cards = malloc (CARDN * sizeof (uint));
	player->cardn = CARDN; //CPPLAYER;
	player->cardi = 0;

	for (uint i = 1; i < botn; i++) {
		bot = &playerbuf[i];
		bot->tag = BOT;
		bot->bot = NULL; // TODO
		bot->cards = malloc (CARDN * sizeof (uint));
		bot->cardn = CARDN; // CPPLAYER;
		bot->cardi = 0;
	}
}

static void drawcards (uint botn, uint cardn) { // draw initial cards from the deck
	for (uint cn = 0; cn < CPPLAYER; cn++) {
		for (uint i = 0; i < botn; i++) {
			(void) take (playerringbuf[i], 1);
		}
		ringshuffle (playerringbuf, botn);
	}
}

void popplayers (Deckr* deckr, uint botn, uint cardn) { // populate the bots' cards
	playerbuf = malloc (botn  * sizeof (Player));
	playerringbuf = malloc (botn * sizeof (Player*));
	for (uint i = 0; i < botn; i++) playerringbuf[i] = &playerbuf[i];
	popbots (botn);
	ringshuffle (playerringbuf, botn);
	drawcards (botn, cardn);
	// NOTE: if a `play again?' option is ever added, this buffer will not have to be freed
	free (playerringbuf);
}
