#include <stdlib.h>

#include "nou.h"
#include "deck.h"
#include "players.h"

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

Card* stacktop = NULL, * stackbase = NULL;
Card* decktop = NULL, * deckbase = NULL;

Suit csuit = NOSUIT;
uint acc = 0;

uint seed, reseed;

static Player** playerringbuf;

static uint sort_vacanti (uint* playing, uint* playeroff) {
	Deck* deck = deckr.deck;
	uint cards = deckr.cards;

	Card* card = NULL;
	Player* owner = NULL;

	uint _playing = *playing;
	uint playeri = 0;

	uint lvacant_i = -1u;

	for (uint _ = 0; _ < cards; _++) {
		card = &index (deck, _, cards);
		owner = card->owner;
		if (!owner) {
			lvacant_i = _;
			break;
		}
		else {
			_playing--;
			playeri = (card->owner - player);
			playeroff[playeri]++;
		}
	}

	*playing = _playing;

	return lvacant_i;
}

static void _sort (uint i, uint* playing, uint* playeroff) {
	Deck* deck = deckr.deck;
	uint cards = deckr.cards;

	Card* vacant = NULL, *card = NULL, tmp;

	Player* owner;

	uint _playing = *playing;
	uint playeri = 0;

	for (uint vacant_i = 0; i < cards; i++) {
		card = &index (deck, i, cards);
		owner = card->owner;

		// found a vacant card: mark it
		if (!owner && !vacant) {
			vacant_i = i;
			vacant = card;
		}

		// found an owned card: ...
		else if (owner) {
			_playing--;

			// ... if there is a vacant card before it, recompute the index then swap ...
			if (vacant) {
				playeri = (card->owner - player);
				owner->cards[playeroff[playeri]] = (vacant - deckbase);
				playeroff[playeri]++;

				tmp = *card;
				*card = *vacant;
				*vacant = tmp;

				vacant = NULL;
				i = vacant_i;
			}

			// ... otherwise, save the index as normal
			else {
				playeri = (card->owner - player);
				owner->cards[playeroff[playeri]] = (card - deckbase);
				playeroff[playeri]++;
			}
		}
		// short-circuit
		else if (! _playing) {
			break;
		}
	}

	*playing = _playing;
}

int sort (void) {
	Deck* deck = deckr.deck;
	uint cards = deckr.cards, playing = deckr.playing;

	uint playeroff[playern];

	uint lvacant_i = -1u;
	uint stackbase_i = (cards - 1);

	for (uint _ = 0; _ < playern; _++) {
		playeroff[_] = 0;
	}

	lvacant_i = sort_vacanti (&playing, (uint*)playeroff);

	if (lvacant_i == -1u) {
		return NOCARDS;
	}

	_sort (lvacant_i, &playing, (uint*)playeroff);

	// NOTE: this assumes the last index is vacant
	while (!index (deck, stackbase_i, cards).owner) {
		stackbase_i--;
	}

	stackbase = (deckbase + stackbase_i) + 1;

	// account for the increment in `take_from_stack'
	stacktop = stackbase - 1;

	return 0;
}

void popdeck (Deckr* deckr) {
	Card card;
	Deck* deck = deckr->deck;
	uint n = deckr->n;
	uint i = 0, c = 0;

iter:
	for (Suit suit = DIAMONDS; suit <= SPADES; suit++)
		for (Number number = _A; number <= _J; number++) {
			card.suit = suit;
			card.number = number;
			card.owner = NULL;
			deck[i][c] = card;
			c++;
		}

	//c++;
	card.suit = SPECIAL;
	card.number = _B;
	card.owner = NULL;
	deck[i][c] = card;
	c++;

	card.suit = SPECIAL;
	card.number = _C;
	card.owner = NULL;
	deck[i][c] = card;
	c = 0;

	i++;
	if (i != n) {
		goto iter;
	}
};

uint reseedr (uint primr) {
	if (! primr) {
		primr = seed + reseed;
	}

	uint div = primr < seed? (seed / primr): (primr / seed);
	uint reseedbits = div % (sizeof reseed);

	reseed <<= reseedbits;
	reseed |= ONES (reseedbits);

	return seed ^ reseed;
}

uint seedr (uint primr) {
	if (! primr) {
		primr = seed + reseed;
	}

	uint div = primr < seed? (seed / primr): (primr / seed);
	uint reseedbits = div % (sizeof reseed);

	reseed |= seed;
	reseed <<= reseedbits;
	reseed ^= seed;

	return reseedr (reseed+1);
}

uint seeded (uint n) {
	uint ret = seed % n;
	seed = seedr (ret);

	return ret;
}

static void swap (Deck* deck, uint i, uint pcards) {
	uint _i = seeded (pcards - i) + i;

	if (_i == i) {
		return;
	}

	Card tmp = index (deck, i, pcards);
	index (deck, i, pcards) = index (deck, _i, pcards);
	index (deck, _i, pcards) = tmp;
}

void shuffle (Deckr* deckr, uint pcards) {
	Deck* deck = deckr->deck;
	for (uint i = 0; i < pcards; i++) {
		swap (deck, i, pcards);
	}
}

static void ringswap (Player** ring, uint i, uint botn) {
	uint _i = seeded (botn - i) + i;

	if (_i == i) {
		return;
	}

	Player* tmp = ring[i];
	ring[i] = ring[_i];
	ring[_i] = tmp;
}

static inline void ringshuffle (Player** ring, uint botn) {
	for (uint i = 0; i < botn; i++) {
		ringswap (ring, i, botn);
	}
}

static void allocplayers (uint botn) {
	Player* bot;

	player = &playerbuf[0];
	player->tag = PLAYER;
	player->cards = malloc (CARDN * sizeof (uint));
	player->cardn = CARDN;
	player->cardi = 0;

	for (uint i = 1; i < botn; i++) {
		bot = &playerbuf[i];
		//init_bot (0.0f, i);
		bot->tag = BOT;
		bot->cards = malloc (CARDN * sizeof (uint));
		bot->cardn = CARDN;
		bot->cardi = 0;
	}
}

static void drawcards (uint botn, uint cardn) {
	for (uint cn = 0; cn < CPPLAYER; cn++) {
		ringshuffle (playerringbuf, botn);
		for (uint i = 0; i < botn; i++) {
			(void) take (playerringbuf[i], 1, ALWAYS);
		}
	}
}

void popplayers (Deckr* deckr, uint botn, uint cardn) {
	playerbuf = malloc (botn * sizeof (Player));
	botbuf = malloc ((botn - 1) * sizeof (Bot));

	playerringbuf = malloc (botn * sizeof (Player*));

	for (uint i = 0; i < botn; i++) {
		playerringbuf[i] = &playerbuf[i];
	}

	stackbase = deckbase;
	allocplayers (botn);
	drawcards (botn, cardn);

	free (playerringbuf);
}
