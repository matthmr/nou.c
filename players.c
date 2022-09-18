#include <stdlib.h>
#include <unistd.h>

#if DEBUG == 1
#include <stdio.h>
#endif

#include "players.h"
#include "nou.h"
#include "cmd.h"
#include "deck.h"

Player* playerbuf = NULL, * player = NULL;
uint playern = 0;

Legal** legalbuf = NULL;

// NOTE: this strategy only applies as either a conjunction to
// `bsuit' (most populated suit) or as a random fallback
struct ldup {
	uint nam;
	Suit sam;

	Number num;
};

static uint ldup_sort_legal (Legal*, uint, struct ldup*);
static uint bsuit_sort_legal (Legal*, uint, Suit);
static uint num_sort_legal (Legal*, uint, Number);
static Suit csuit_bias (Card*, Suit);
static uint bsuit_bias (Legal*, uint, Suit, Number, Suit);

static uint on_counteract_bias (Legal*, uint, Suit);
static uint on_disadvantage_bias (Legal*, uint, Suit);
static uint bias (Legal*, uint, Suit);

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

static uint ldup_sort_legal (Legal* legal, uint len, struct ldup* ldupbuf) {
	uint lessof = -1u;
	uint i = 0, ret = 0;

	// populate `ldupbuf'
	for (i = 0; i < len; i++) {
		ldupbuf[legal[i].number].nam++;
		ldupbuf[legal[i].number].sam |= legal[i].suit;
	}

	// sort `ldupbuf'
	for (i = 0; i < 13; i++) {
		uint nam = ldupbuf[i].nam;
		if (nam) {
			if (nam < lessof) {
				ret = 0;
				struct ldup target = ldupbuf[i];
				target.num = (Number) i;

				ldupbuf[i] = ldupbuf[ret];
				ldupbuf[ret] = target;
				lessof = nam;
			}
			else if (ldupbuf[i].nam == lessof) {
				ret++;
				struct ldup target = ldupbuf[i];
				target.num = (Number) i;

				ldupbuf[i] = ldupbuf[ret];
				ldupbuf[ret] = target;
			}
		}
	}

	for (i = 13; i < 15; i++) {
		// BIAS(10): hijack `_sort_legal', send a special card instead
		if (PERC (10) && ldupbuf[i].nam) {
			ret = 0;
			struct ldup target = ldupbuf[i];
			target.num = (Number) i;

			ldupbuf[i] = ldupbuf[ret];
			ldupbuf[ret] = target;
		}
	}

	return ret;
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
static uint bias (Legal* legal, uint len, Suit bsuit) {
	uint ret = 0;
	Number bnum = NONUMBER;

	struct ldup ldupbuf[15] = {{0}};
	struct ldup ldup = {0};
	uint lduplen = 0;

	uint rnd;

	// BIAS(10): it ignores everything and plays a random legal card
	if (PERC (10)) {
		rnd = seeded (len);
		ret = legal[rnd].ci;
		goto done;
	}

	// populate `ldupbuf'
	lduplen = ldup_sort_legal (legal, len, ldupbuf);

	// deal with duplicates
	if (lduplen) {
		lduplen++;
		ldup = ldupbuf[seeded (lduplen)];
	}
	else {
		ldup = ldupbuf[lduplen];
	}

	bnum = ldup.num;

	// BIAS(10): it ignores `bsuit' in favor of the suit of `bnum'
	if (PERC (10)) bias_bnum: {
		uint ldupsuit[5] = {0};
		uint ldupsuitlen = 0;

		// populate `ldupsuit'
		for (uint i = 0; i < 5; i++) {
			if (ldup.sam & BIT (i+1)) {
				ldupsuit[ldupsuitlen] = BIT (i+1);
				ldupsuitlen++;
			}
		}

		rnd = seeded (ldupsuitlen);
		ret = findcard (legal, len, bnum, ldupsuit[rnd]);

		goto done;
	}

	// BIAS(80): it applies `bsuit' in conjunction with `bnum'
	if (PERC (80)) {
		if (ldup.sam & bsuit) {
			ret = findcard (legal, len, bnum, ldup.sam & bsuit);

			goto done;
		}

		// BIAS(80): it opts for `bsuit' only
		else if (PERC (80)) {
			goto bias_bsuit;
		}
		else {
			goto bias_bnum;
		}
	}

	// BIAS(20): it only applies `bsuit'
	else bias_bsuit: {
		uint bsuitlen = bsuit_sort_legal (legal, len, bsuit);
		rnd = seeded (bsuitlen);
		ret = legal[rnd].ci;
		goto done;
	}

done:
	return ret;
}

static uint bsuit_bias (Legal* legal, uint len, Suit bsuit,
												Number number, Suit base) {
	uint ret = 0;

	// BIAS(10): it plays a random card matching the number
	if (PERC (10)) perc: {
		uint li = num_sort_legal (legal, len, number);
		uint rnd = seeded (li);
		ret = legal[rnd].ci;
	}

	// BIAS(90): it plays a card of `bsuit' suit matching the number
	else if (base & bsuit) {
		ret = findcard (legal, len, number, bsuit);
	}
	else {
		goto perc;
	}

	return ret;
}

static Suit csuit_bias (Card* card, Suit bsuit) {
	Suit ret;
	uint rnd = 0;

	if (card->suit == SPECIAL) {
		// BIAS(10): it ignores `bsuit' and chooses a random suit
		if (PERC (10)) {
			rnd = seeded (4);
			ret = (Suit) (BIT (rnd+1));
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

static uint bsuit_sort_legal (Legal* legal, uint len, Suit bsuit) {
	uint j = 0;

	for (uint i = 0; i < len; i++) {
		if (legal[i].suit == bsuit) {
			Legal tmp = legal[j];
			legal[j] = legal[i];
			legal[i] = tmp;
			j++;
		}
	}

	return j;
}

static uint num_sort_legal (Legal* legal, uint len, Number number) {
	uint j = 0;

	for (uint i = 0; i < len; i++) {
		if (legal[i].number == number) {
			Legal tmp = legal[j];
			legal[j] = legal[i];
			legal[i] = tmp;
			j++;
		}
	}

	return j;
}

static uint on_counteract_bias (Legal* legal, uint len, Suit bsuit) {
	struct ca_bias {
		Suit b, j, q;
	};

	struct ca_bias ca_bias = {
		.b = NOSUIT,
		.j = NOSUIT,
		.q = NOSUIT,
	};

	uint ret = 0;

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
		ret = bsuit_bias (legal, len, bsuit,
											_Q, ca_bias.q);
	}
	else if (ca_bias.j) bias_j: {
		ret = bsuit_bias (legal, len, bsuit,
											_J, ca_bias.j);
	}
	else if (ca_bias.b) bias_b: {
		ret = findcard (legal, len, _B, SPECIAL);
	}

	return ret;
}

static uint on_disadvantage_bias (Legal* legal, uint len, Suit bsuit) {
	uint ret = 0;

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
		ret = bsuit_bias (legal, len, bsuit,
											_K, da_bias.k);
	}
	else if (da_bias.q) bias_q: {
		ret = bsuit_bias (legal, len, bsuit,
											_Q, da_bias.q);
	}
	else if (da_bias.j) bias_j: {
			ret = bsuit_bias (legal, len, bsuit,
												_J, da_bias.j);
	}

	else if (da_bias.c) bias_c: {
		ret = findcard (legal, len, _C, SPECIAL);
	}
	else if (da_bias.b) bias_b: {
		ret = findcard (legal, len, _B, SPECIAL);
	}
	else {
		ret = bias (legal, len, bsuit);
	}

	return ret;
}

static Suit bestsuit (Legal* legal, uint len) {
	Suit ret = NOSUIT;
	Suit suit;

	Suit sam[5] = {0};
	Suit ssam[5] = {0};
	uint moreof = 0, j = -1u;

	// populate `cam'
	for (uint i = 0; i < len; i++) {
		suit = legal[i].suit;
		// dirty hack to map suits to indices
		sam[(suit>>(1+(suit & 16))) - ((suit & 8)>>3)]++;
	}

	// get the most populous suit
	for (uint i = 0; i < 5; i++) {
		if (sam[i] > moreof) {
			ssam[0] = sam[i];
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

static uint bot_choose_card (Player* bot, Legal* legal, uint len) {
	Card* card = NULL;
	uint i = 0;

	Player* np = turn_after ((bot - player), 1);

	// BIAS: it only knows about its own cards
	Suit bsuit = bestsuit (legal, len);

	// BIAS: it handles having only one legal card differently
	if (len == 1) {
		i = legal[0].ci;
		card = &index (deckr.deck, bot->cards[i], deckr.cards);
		Number cnum = card->number;

		if (card->suit == SPECIAL && PERC (90)) {
			// BIAS(10): it plays a special card if it is the only legal card
			if (PERC (90)) {
				return -1u;
			}
			else {
				goto done;
			}
		}

		// BIAS(40): it plays an action card if it is the only legal card
		else if (cnum == _K || cnum == _Q || cnum == _J) {
			if (PERC (60)) {
				return -1u;
			}
			else {
				goto done;
			}
		}
	}

	// BIAS: it always try to counteract
	if (acc) {
		i = on_counteract_bias (legal, len, bsuit);
	}

	else if (np->cardi < bot->cardi) {
		// BIAS(80): it prefers specials on numerial disadvantage
		if (PERC (80)) {
			i = on_disadvantage_bias (legal, len, bsuit);
		}
		else {
			goto default_bias;
		}
	}
	
	// BIAS: it applies some internal bias to choose a strategy to play in
	else default_bias: {
		i = bias (legal, len, bsuit);
	}

	card = &index (deckr.deck, bot->cards[i], deckr.cards);
	csuit = csuit_bias (card, bsuit);

done:
	return i;
}

static uint poplegal (Player* bot, Legal* botlegal) {
	Deck* deck = deckr.deck;
	uint cards = deckr.cards;

	uint* bcards = bot->cards;
	uint cardi = bot->cardi;

	uint j = 0;

	for (uint i = 0; i < cardi; i++) {
		Card card = index (deck, bcards[i], cards);

		if (legal (card, *top)) {
			botlegal[j].suit = card.suit;
			botlegal[j].number = card.number;
			botlegal[j].ci = i;
			j++;
			adjust_legalbuf (bot, j);
		}
	}

	return j;
}
// -- bot stack bottom -- //

void bot_play (Cmd* cmd, uint boti) {
	Player* bot = &playerbuf[boti];
	Legal* botlegal = legalbuf[boti-1];

	uint j = poplegal (bot, botlegal);

	#if DEBUG == 1
	for (uint i = 0; i < j; i++) {
				 Card debugcard = index (deckr.deck,
																 cmd->p->cards[botlegal[i].ci],
																 deckr.cards);
				 fprintf (stderr, "[bot_play::legal] \n lcard.number = %d \n lcard.suit = %d \n",
									(int) debugcard.number,
									(int) debugcard.suit);
	}
	#endif

	// apply action
	if (!j) bot_take: {
		cmd->ac.cmd = TAKE;
		cmd->status = (acc? CACC: COK);
		cmd->ac.am = 1;
	}
	else {
		uint i = bot_choose_card (bot, botlegal, j);

		if (i == -1u) {
			goto bot_take;
		}

		cmd->ac.cmd = PLAY;
		cmd->status = COK;

		// we have to play like a human
 		cmd->ac.target = i + 1;
	}

	usleep (PLAY_INTERVAL);

	#if DEBUG == 1
	Card debugcard = index (deckr.deck,
													cmd->p->cards[cmd->ac.target-1],
													deckr.cards);
	fprintf (stderr, "\n[bot_play::chosen] \n pcard.number = %d \n pcard.suit = %d \n",
					 (int) debugcard.number,
					 (int) debugcard.suit);
  #endif
}

void bot_init (uint boti) {
	legalbuf[boti-1] = malloc (CARDN * sizeof (Legal));
}
