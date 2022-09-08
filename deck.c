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
Card* card0 = NULL;
Card* stacktop = NULL, * decktop = NULL;

Suit csuit = NOSUIT;
uint acc = 0;

uint seed, reseed;

static Player** playerringbuf;

int sort (Deckr* deckr) {
	Deck* deck = deckr->deck;
	uint cards = deckr->cards, playing = deckr->playing;

	Card* vacant = NULL, *card = NULL, tmp;
	uint lvacant_i = -1u;

	Player* owner;
	uint playeroff[playern], playeri;

	for (uint _ = 0; _ < playern; _++)
		playeroff[_] = 0;

	for (uint _ = 0; _ < cards; _++) {
		card = &index (deck, _, cards);
		owner = card->owner;
		if (!owner) {
			lvacant_i = _;
			break;
		}
		else {
			playing--;
			playeri = (card->owner - player);
			playeroff[playeri]++;
		}
	}
	
	if (lvacant_i == -1u)
		return NOCARDS;

	for (uint i = lvacant_i, vacant_i = 0; i < cards; i++) {
		card = &index (deck, i, cards);
		owner = card->owner;

		if (!owner && !vacant) {
			vacant_i = i;
			vacant = card;
		}
		else if (owner) {
			playing--;

			if (vacant) {
				playeri = (card->owner - player);
				owner->cards[playeroff[playeri]] = (card - card0) - 1;
				playeroff[playeri]++;

				tmp = *card;
				*card = *vacant;
				*vacant = tmp;

				vacant = NULL;
				i = vacant_i;
				continue;
			}
			else {
				playeri = (card->owner - player);
				owner->cards[playeroff[playeri]] = (card - card0);
				playeroff[playeri]++;
			}
		}
		else if (! playing) break;
	}

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
	if (i != n) goto iter;
	card0 = &deckr->deck[0][0];
};

uint reseedr (uint primr) {
	if (! primr) primr = seed + reseed;
	uint div = primr < seed? (seed / primr): (primr / seed);
	uint reseedbits = div % (sizeof reseed);
	reseed <<= reseedbits;
	reseed |= ONES (reseedbits);
	return seed ^ reseed;
}

uint seedr (uint primr) {
	if (! primr) primr = seed + reseed;
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

void swap (Deck* deck, uint i, uint pcards) {
	//uint _i = seeded (pcards) + i;
	uint _i = (seeded (pcards) + i) % pcards;
	if (! _i) return;
	Card tmp = index (deck, i, pcards);
	index (deck, i, pcards) = index (deck, _i, pcards);
	index (deck, _i, pcards) = tmp;
}

void shuffle (Deckr* deckr, uint pcards) {
	//TODO: `pcards' should shift the shufflable deck up by itself
	Deck* deck = deckr->deck;
	for (uint i = 0; i < pcards; i++)
		swap (deck, i, pcards);
}

static void ringswap (Player** ring, uint i, uint botn) {
	//uint _i = seeded (botn) + i;
	uint _i = (seeded (botn) + i) % botn;
	if (! _i) return;
	Player* tmp = ring[i];
	ring[i] = ring[_i];
	ring[_i] = tmp;
}

static inline void ringshuffle (Player** ring, uint botn) {
	for (uint i = 0; i < botn; i++)
		ringswap (ring, i, botn);
}

static void popbots (uint botn) {
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
		for (uint i = 0; i < botn; i++) {
			(void) take (playerringbuf[i], 1, true);
		}
		ringshuffle (playerringbuf, botn);
	}
}

void popplayers (Deckr* deckr, uint botn, uint cardn) { // populate the bots' cards
	playerbuf = malloc (botn * sizeof (Player));
	botbuf = malloc ((botn - 1) * sizeof (Bot));

	playerringbuf = malloc (botn * sizeof (Player*));

	for (uint i = 0; i < botn; i++)
		playerringbuf[i] = &playerbuf[i];

	popbots (botn);
	ringshuffle (playerringbuf, botn);
	drawcards (botn, cardn);

	free (playerringbuf);
}
