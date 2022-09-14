#include <stdlib.h>

#include "players.h"
#include "nou.h"
#include "cmd.h"

Player* playerbuf = NULL, * player = NULL;
uint playern = 0;

// TODO: add a difficulty flag with gpld.argparser

// -- bot stack top -- //
static Player* turn_after (uint i, uint step) {
	i += ((dir == DIRCLOCKWISE)? 1: (playern-1))*step;
	i %= playern;

	return &playerbuf[i];
}

static Card* counteract (Legal* legal, uint len) {
	Card* ret = NULL;

	for (uint i = 0; i < len; i++) {
		Number number = legal[i].number;

		struct {
			uint b;
			Suit j[4], q[4];
		} bias;

		bias = {.j = -1u, .q = -1u, .b = -1u};

		if (number >= _Q && number <= _B) {
			switch (number) {
			case _Q: bias.q[legal[i].suit] = legal[i].i; break;
			case _J: bias.j[legal[i].suit] = legal[i].i; break;
			case _B: bias.b = legal[i].i; break;
			}
		}
	}

	// BIAS: prefer _Q over _J over _B
	if (~bias.q) {
	}
	else if (~bias.j) {
	}
	else if (~bias.b) {
		card = &index (deckr.deck, bias.b, deckr.cards);
	}

	return card;
}

static Card* ndisadvantage (Legal* legal, uint len) {
	Card* ret = NULL;

	for (uint i = 0; i < len; i++) {
		Number number = legal[i].number;

		struct {
			uint b, c;
			Suit j[4], q[4];
		} bias;

		bias = {.j = -1u, .q = -1u, .b = -1u};

		if (number >= _Q && number <= _C) {
			switch (number) {
			case _Q: bias.q[legal[i].suit] = legal[i].i; break;
			case _J: bias.j[legal[i].suit] = legal[i].i; break;
			case _B: bias.b = legal[i].i; break;
			case _C: bias.c = legal[i].i; break;
			}
		}
	}

	return card;
}

static Suit bestsuit (Legal* legal, uint len) {
	Suit ret = NOSUIT;

	uint cam[5];
	uint moreof = 0;

	// populate `cam'
	for (uint i = 0; i < len; i++) {
		cam[legal[i].suit]++;
	}

	// get the most populous suit
	for (uint i = 0; i < 5; i++) {
		if (cam[i] > moreof) {
			moreof = cam[i];
			ret = i;
		}
	}

	return ret;
}

static Card* bot_think_play (Player* bot, Legal* legal, uint len) {
	Card* ret = NULL;
	Player* np = turn_after ((bot - player), 1);

	// BIAS: it only knows about its own cards
	Suit bsuit = bestsuit (legal, len);

	// BIAS: it always try to counteract
	if (acc) {
		ret = counteract (legal, len);
		if (ret) {
			goto done;
		}
	}

	// BIAS: it prefer specials on numerial disadvantage
	if (np.cardi < bot.cardi) {
		ret = ndisadvantage (legal, len);
		if (ret) {
			goto done;
		}
	}

	// BIAS: it plays the card with the least duplicate 

done:
	return ret;
}

static void adjust_legalbuf (Player* bot) {
	if ((bot->cardi+1) == bot->cardn) {
		uint boti = (bot - player);
		legalbuf[boti] = realloc (legalbuf[boti], boti * sizeof (Player));
	}
	return;
}
// -- bot stack bottom -- //

void bot_play (Cmd* cmd, uint boti) {
	Player* bot = playerbuf[boti];
	Legal* legal = legalbuf[boti-1];

	uint* cards = bot->cards;

	Deck deck = deckr.deck;
	uint cards = deckr.cards;

	uint cardi = bot->cardi;
	uint j = 0;

	// populate `legalbuf'
	for (uint i = 0; i < cardi; i++) {
		Card card = index (deck, bot->cards[i], card);

		if (legal (card, *top)) {
			legal[j].suit = card.suit;
			legal[j].number = card.number;
			legal[i].i = i;
			j++;
		}
	}

	// take action
	if (!j) {
		cmd->ac.cmd = TAKE;
		cmd->ac.am = 1;
		cmd->status = (acc? CACC: COK);
		adjust_legalbuf (bot);
	}
	else {
		Card* pcard = bot_think_play (bot, legal, j);

		cmd->ac.cmd = PLAY;
		cmd->target = (pcard - deckbase);
		cmd->status = COK;
	}
}

void bot_init (uint boti) {
	legalbuf[boti-1] = malloc (CARDN * sizeof (Legal));
}
