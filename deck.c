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

static uint find_vacanti (uint* playing, uint* playeroff) {
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

#define MARKVACANT(x,y) (!(x) && !(y))
static void sort_deck (uint i, uint* playing, uint* playeroff) {
	Deck* deck = deckr.deck;
	uint cards = deckr.cards;

	Card* vacant = NULL, *card = NULL, tmp;

	Player* owner;

	uint _playing = *playing;
	uint playeri = 0;

	for (; i < cards; i++) {
		card = &index (deck, i, cards);
		owner = card->owner;

		// found a vacant card: mark it
		if (MARKVACANT (owner, vacant)) {
			vacant = card;
		}

		// found an owned card: ...
		else if (owner) {
			_playing--;

			// ... if there is a vacant card before it, recompute the index then swap ...
			if (vacant) {
				playeri = (owner - player);
				owner->cards[playeroff[playeri]] = (vacant - deckbase);
				playeroff[playeri]++;

				tmp = *card;
				*card = *vacant;
				*vacant = tmp;

				if (vacant != decktop && !(vacant+1)->owner) {
					vacant++;
				}
				else {
					vacant = NULL;
				}
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

	lvacant_i = find_vacanti (&playing, (uint*)playeroff);

	// there are no vacant cards left
	if (lvacant_i == -1u) {
		return NOCARDS;
	}

	sort_deck (lvacant_i, &playing, (uint*)playeroff);

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
	for (uint suitn = 1; suitn <= 4; suitn++)
		for (Number number = _A; number <= _J; number++) {
			card.suit = BIT (suitn);
			card.number = number;
			card.owner = NULL;
			deck[i][c] = card;
			c++;
		}

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

uint reseeder (uint primr) {
	uint _primr = primr;

reseeder_primr:	
	if (! primr) {
		primr += (reseed ^ _primr);
		primr++;
		_primr++;
		goto reseeder_primr;
	}
	else {
reseeder_seed:
		if (! seed) {
			seed ^= reseed;
			seed++;
			goto reseeder_seed;
		}
	}

	uint div = primr < seed? (seed / primr): (primr / seed);
	uint reseedbits = div % (sizeof reseed);

	reseed <<= reseedbits;
	reseed |= ONES (reseedbits);

	return seed ^ reseed;
}

uint seeder (uint primr) {
	uint _primr = primr;

seeder_primr:	
	if (! primr) {
		primr += (reseed ^ _primr);
		primr++;
		_primr++;
		goto seeder_primr;
	}
	else {
seeder_seed:
		if (! seed) {
			seed ^= reseed;
			seed++;
			goto seeder_seed;
		}
	}

	uint div = primr < seed? (seed / primr): (primr / seed);
	uint reseedbits = div % (sizeof reseed);

	reseed |= seed;
	reseed <<= reseedbits;
	reseed ^= seed;

	return reseeder (seed+reseed+1);
}

uint seeded (uint n) {
	if (!n ) {
		return n;
	}

	uint ret = 0;
	seed = seeder (seed+reseed+1);

	ret = (seed + reseed) % n;

	reseeder (ret);

	return ret;
}

static void swap (Deck* deck, uint i, uint cards, uint pcards) {
	uint swp = (cards - pcards) + i;
	uint _swp = seeded (cards - swp) + swp;

	if (swp == _swp) {
		return;
	}

	Card tmp = index (deck, swp, cards);
	index (deck, swp, cards) = index (deck, _swp, cards);
	index (deck, _swp, cards) = tmp;
}

void shuffle (Deckr* deckr, uint cards, uint pcards) {
	Deck* deck = deckr->deck;

	// loop through the playable cards
	for (uint i = 0; i < pcards; i++) {
		swap (deck, i, cards, pcards);
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
		bot_init (i);
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
	legalbuf = malloc ((botn - 1) * sizeof (Legal*));
	playerringbuf = malloc (botn * sizeof (Player*));

	for (uint i = 0; i < botn; i++) {
		playerringbuf[i] = &playerbuf[i];
	}

	stackbase = deckbase;
	allocplayers (botn);
	drawcards (botn, cardn);

	free (playerringbuf);
}
