#include <stdlib.h>
#include <unistd.h>

#include "players.h"
#include "nou.h"
#include "cmd.h"
#include "deck.h"

Player* playerbuf = NULL, * player = NULL;
uint playern = 0;

Legal** legalbuf = NULL;

// biases
static Card* rndsuit_bias (Legal**, uint, Number, Suit, Suit);
static uint bsuit_sort_bias (Legal**, uint, Suit);
static uint num_sort_bias (Legal**, uint, Number);
static Suit suit_bias (Card*, Suit);
static Card* on_counteract_bias (Legal*, uint, Suit);
static Card* on_disadvantage_bias (Legal*, uint, Suit);
static Card* bias (Legal*, uint, Suit);

// -- bot stack top -- //
static inline uint findcard (Legal* legal, uint len, Number number, Suit suit) {
	uint ret = 0;

	for (uint i = 0; i < len; i++) {
		if (legal[i].number == number && legal[i].suit == suit) {
			ret = legal[i].ci;
			break;
		}
	}

	return ret;
}

static inline void adjust_legalbuf (Player* bot, uint len) {
	static uint prevlim = CARDN;

	// NOTE: we adjust the legal buffer in a slower rate than the overall card buffer
	if (len == prevlim) {
		uint boti = (bot - player);
		legalbuf[boti] = realloc (legalbuf[boti], (prevlim += CARDN) * sizeof (Legal));
	}
}

static inline Player* turn_after (uint i, uint step) {
	i += ((dir == DIRCLOCKWISE)? 1: (playern-1))*step;
	i %= playern;

	return &playerbuf[i];
}

/*
 * The default strategy is to play a card in the most populous suit
 * that has the least ammount of duplicates in other suits, but it
 * also applies some random bias against overturning one of these
 * strategies
 *
 * Base strategies change with different implementations of these
 * family of functions. They all end with `_bias'.
 */
static Card* bias (Legal* legal, uint len, Suit bsuit) {
	Card* card = NULL;
	
	struct ldup {
		uint nam;
		Suit sam;
	};

	struct ldup ldupbuf[13] = {{0}};
	struct ldup ldup = {0};

	uint rnd;

	// BIAS(10): it ignores everything and plays a random card
	if (PERC (10)) {
		rnd = seeded (len);
		card = &index (deckr.deck, legal[rnd].ci, deckr.cards);
		csuit = suit_bias (card, bsuit);
		goto done;
	}

	// populate `ldupbuf'
	for (uint i = 0; i < len; i++) {
		ldupbuf[legal[i].number].nam++;
		ldupbuf[legal[i].number].sam |= legal[i].suit;
	}

	uint _nam = -1u;

	// find `ldup'
	for (uint i = 0; i < 13; i++) {
		if (ldupbuf[i].nam && ldupbuf[i].nam < _nam) {
			_nam = ldupbuf[i].nam;
			ldup.nam = (Number) i;
			ldup.sam = ldupbuf[i].sam;
		}
	}

	// BIAS(15): it ignores `bsuit' in favor of the suit of `ldup'
	if (PERC (15)) bias_ldup: {
		uint ldupsuit[4];
		uint ldupsuitlen = 0;

		// populate `ldupsuit'
		for (uint i = 0; i < 4; i++) {
			if (ldup.sam & BIT (i+1)) {
				ldupsuit[ldupsuitlen] = BIT (i+1);
				ldupsuitlen++;
			}
		}

		rnd = seeded (ldupsuitlen);
		card = &index (deckr.deck,
									 findcard (legal, len, ldup.nam, ldupsuit[rnd]),
									 deckr.cards);
		csuit = suit_bias (card, bsuit);
		goto done;
	}

	// BIAS(80): it applies `bsuit' in conjunction with `ldup'
	if (PERC (80)) {
		if (ldup.sam & bsuit) {
			card = &index (deckr.deck,
										 findcard (legal, len, ldup.nam, ldup.sam & bsuit),
										 deckr.cards);
			goto done;
		}
		else {
			goto bias_ldup; 
		}
	}

	// BIAS(20): it only applies `bsuit'
	else {
		uint bsuitlen = bsuit_sort_bias (&legal, len, bsuit);
		rnd = seeded (bsuitlen);
		card = &index (deckr.deck, legal[rnd].ci, deckr.cards);
		goto done;
	}

done:
	return card;
}

static Card* rndsuit_bias (Legal** legal,
													 uint len,
													 Number number,
													 Suit bsuit,
													 Suit base) {
	Card* card = NULL;
	Legal* _legal = *legal;
	uint rnd = 0;

	if (PERC (10)) perc: {
		uint qn = num_sort_bias (legal, len, number);
		rnd = seeded (qn);
		card = &index (deckr.deck, _legal[rnd].ci, deckr.cards);
	}
	else if (base & bsuit) {
			card = &index (deckr.deck,
										 findcard (_legal, len, number, bsuit),
										 deckr.cards);
	}
	else {
		goto perc;
	}

	return card;
}

static Suit suit_bias (Card* card, Suit bsuit) {
	Suit ret;
	uint rnd = 0;

	if (card->suit == SPECIAL) {
		// BIAS(10): it ignores `bsuit' and chooses a random suit
		if (PERC (10)) {
			rnd = seeded (4);
			ret = (Suit) BIT (rnd+1);
		}
		else {
			ret = bsuit;
		}
	}
	else {
		ret = csuit;
	}

	return ret;
}

static uint bsuit_sort_bias (Legal** legal, uint len, Suit bsuit) {
	uint i = 0, j = 0;
	Legal* _legal = *legal;

	for (; i < len; i++) {
		if (_legal[i].suit == bsuit) {
			_legal[j] = _legal[i];
			j++;
		}
	}

	return j;
}

static uint num_sort_bias (Legal** legal, uint len, Number number) {
	uint i = 0, j = 0;
	Legal* _legal = *legal;

	for (; i < len; i++) {
		if (_legal[i].number == number) {
			_legal[j] = _legal[i];
			j++;
		}
	}

	return j;
}

static Card* on_counteract_bias (Legal* legal, uint len, Suit bsuit) {
	Card* card = NULL;

	struct ca_bias {
		Suit b, j, q;
	};

	struct ca_bias ca_bias = {
		.b = NOSUIT,
		.j = NOSUIT,
		.q = NOSUIT,
	};

	// populate `ca_bias'
	for (uint i = 0; i < len; i++) {
		Number number = legal[i].number;
		Suit suit = legal[i].suit;

		switch (number) {
		case _Q:
			ca_bias.q |= suit;
			break;
		case _J:
			ca_bias.j |= suit;
			break;
		case _B:
			ca_bias.b |= suit;
			break;
		}
	}

	// BIAS: it can jump ignore the next bias
	if (PERC (20) && ca_bias.j) {
		goto bias_j;
	}
	else if (PERC (5) && ca_bias.b) {
		goto bias_b;
	}

	// BIAS: it prefers _Q over _J over _B
	if (ca_bias.q) {
		card = rndsuit_bias (&legal, len, _Q, bsuit, ca_bias.q);
	}
	else if (ca_bias.j) bias_j: {
		card = rndsuit_bias (&legal, len, _J, bsuit, ca_bias.j);
	}
	else if (ca_bias.b) bias_b: {
		card = &index (deckr.deck,
									 findcard (legal, len, _B, SPECIAL),
									 deckr.cards);
		csuit = suit_bias (card, bsuit);
	}

	return card;
}

static Card* on_disadvantage_bias (Legal* legal, uint len, Suit bsuit) {
	Card* card = NULL;

	struct da_bias {
		Suit c, b, j, q, k;
	};

	struct da_bias da_bias = {
		.c = NOSUIT,
		.b = NOSUIT,
		.j = NOSUIT,
		.q = NOSUIT,
		.k = NOSUIT,
	};

	// populate `da_bias'
	for (uint i = 0; i < len; i++) {
		Number number = legal[i].number;
		Suit suit = legal[i].suit;

		switch (number) {
		case _K:
			da_bias.k |= suit;
			break;
		case _Q:
			da_bias.q |= suit;
			break;
		case _J:
			da_bias.j |= suit;
			break;
		case _B:
			da_bias.b |= suit;
			break;
		case _C:
			da_bias.c |= suit;
			break;
		}
	}

	// BIAS: it can jump ignore the next bias
	if (PERC (30) && da_bias.q) {
		goto bias_q;
	}
	else if (PERC (20) && da_bias.j) {
		goto bias_j;
	}
	else if (PERC (10) && da_bias.c) {
		goto bias_c;
	}
	else if (PERC (5) && da_bias.b) {
		goto bias_b;
	}

	if (da_bias.k) {
		card = rndsuit_bias (&legal, len, _K, bsuit, da_bias.k);
	}
	else if (da_bias.q) bias_q: {
		card = rndsuit_bias (&legal, len, _Q, bsuit, da_bias.q);
	}
	else if (da_bias.j) bias_j: {
		card = rndsuit_bias (&legal, len, _J, bsuit, da_bias.j);
	}

	else if (da_bias.c) bias_c: {
		card = &index (deckr.deck,
									 findcard (legal, len, _B, SPECIAL),
									 deckr.cards);
		csuit = suit_bias (card, bsuit);
	}
	else if (da_bias.b) bias_b: {
		card = &index (deckr.deck,
									 findcard (legal, len, _B, SPECIAL),
									 deckr.cards);
		csuit = suit_bias (card, bsuit);
	}
	else {
		return bias (legal, len, bsuit);
	}

	return card;
}

static Suit bestsuit (Legal* legal, uint len) {
	Suit ret = NOSUIT;
	Suit suit;

	uint sam[5] = {0};
	Suit ssam[5] = {0};
	uint moreof = 0, j = -1u;

	// populate `cam'
	for (uint i = 0; i < len; i++) {
		suit = legal[i].suit;
		sam[(suit>>1) - ((suit & 8)>>3)]++;
	}

	// get the most populous suit
	for (uint i = 0; i < 5; i++) {
		if (sam[i] > moreof) {
			moreof = sam[i];
			ret = BIT (i+1);
			j = -1u;
		}
		else if (sam[i] == moreof) {
			j++;
			ssam[j] = BIT (i+1);
		}
	}

	// BIAS: it randomly chooses a suit if it has the same ammount
	if (j != -1u) {
		j++;
		uint i = seeded (j);
		ret = ssam[i];
	}

	return ret;
}

static Card* bot_choose_card (Player* bot, Legal* legal, uint len) {
	Card* card = NULL;
	Player* np = turn_after ((bot - player), 1);

	// BIAS: it only knows about its own cards
	Suit bsuit = bestsuit (legal, len);

	// BIAS: it always try to counteract
	if (acc) {
		card = on_counteract_bias (legal, len, bsuit);
		goto done;
	}

	if (np->cardi < bot->cardi) {
		// BIAS(80): it prefers specials on numerial disadvantage
		if (PERC (80)) {
			card = on_disadvantage_bias (legal, len, bsuit);
		}
		else {
			goto default_bias;
		}
		goto done;
	}
	
	// BIAS: it applies some internal bias to choose a strategy to play in
	else default_bias: {
		card = bias (legal, len, bsuit);
		goto done;
	}
		
done:
	return card;
}
// -- bot stack bottom -- //

void bot_play (Cmd* cmd, uint boti) {
	Player* bot = &playerbuf[boti];
	Legal* blegal = legalbuf[boti-1];

	uint* bcards = bot->cards;

	Deck* deck = deckr.deck;
	uint cards = deckr.cards;

	uint cardi = bot->cardi;
	uint i = 0, j = 0;

	// populate `legalbuf'
	for (; i < cardi; i++) {
		Card card = index (deck, bcards[i], cards);

		if (legal (card, *top)) {
			blegal[j].suit = card.suit;
			blegal[j].number = card.number;
			blegal[j].ci = i;
			j++;
			adjust_legalbuf (bot, j);
		}
	}

	// apply action
	if (!j) action_take: {
		cmd->ac.cmd = TAKE;
		cmd->status = (acc? CACC: COK);
		cmd->ac.am = 1;
	}
	else {
		Card* card = bot_choose_card (bot, blegal, j);
		
		if (! card) {
			goto action_take;
		}

		cmd->ac.cmd = PLAY;
		cmd->status = COK;

		// we have to play like a human
 		cmd->ac.target = (card - deckbase) + 1;
	}

	// TODO: make this configurable via gpld.argparser
	usleep (PLAY_INTERVAL);
}

void bot_init (uint boti) {
	legalbuf[boti-1] = malloc (CARDN * sizeof (Legal));
}
