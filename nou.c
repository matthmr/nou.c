#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "draw.h"
#include "cli.h"
#include "nou.h"
#include "cmd.h"

#define _SLAVE_OK 0

char* cmdbuff = NULL;

static bool block = false, reverse = false;

bool legal (Card playing, Card played) {
	Suit suit = played.suit, psuit = playing.suit;
	Number num = played.number, pnum = playing.number;

	if (num == _C) {
		return (psuit == csuit || psuit == SPECIAL);
	}
	else if (num == _B) {
		return ((psuit == csuit && (pnum == _Q || pnum == _J)) ||
			(psuit == SPECIAL && pnum == _B));
	}
	else if (num == _J) {
		return ((psuit == suit && pnum == _Q) ||
			(pnum == _J) ||
			(psuit == SPECIAL && pnum == _B));
	}

	return (pnum == num || psuit == suit || psuit == SPECIAL);
}

static inline Card* take_from_stack (void) {
	if (! stacktop) {
		stacktop = stackbase;
	}
	else {
		stacktop++;
	}

	if (stacktop > decktop) {
		stacktop = NULL;
	}

	return stacktop;
}

// NOTE: the `::cardi' field is human-readable, so its index
// is taken raw. For example, a value of 7 means the 8th element
// is empty
static inline void player_append (Player* p, Card* card) {
	// TODO: smart memory cleaning; it's not needed now because
	// even the most leaky allocs are only about ~4KB in size
	if (p->cardi == p->cardn) {
		p->cardn += CARDN;
		p->cards = realloc (p->cards,	p->cardn * sizeof (uint));
	}
	p->cards[p->cardi] = (card - deckbase);
	p->cardi++;
}

static int drawfirstcard (void) {
	uint available = (deckr.cards - deckr.playing);
	uint ccardi;

draw:
	// draw the initial card
	ccardi = seeded (available) + deckr.playing;
	Card* ccard = &index (deckr.deck, ccardi, deckr.cards);
	Number dnum = ccard->number;

	// get an actual legal card
	if (ccard->suit == SPECIAL || (
	    dnum == _K || dnum == _Q || dnum == _J
	)) goto draw;
	else {

		(void) take_from_stack ();

		// swap the chosen card against the current stack top
		Card tmp = *stacktop;
		*stacktop = *ccard;
		*ccard = tmp;

		top = stacktop;
		deckr.played++;
	}

	return 0;
}

static Gstat reshuffle (uint* cardi, Card* __top) {
	Card _top = *__top;

	// this implements the "top card must stay on top after reshuffle" rule
	if (deckr.played < 2) {
		//printf ("die here = %d\ndeckr.played = %d\n",	__LINE__,	deckr.played);
		return GDRAW;
	}

	(void) sort ();

	shuffle (&deckr, deckr.played);
	deckr.played = 1;

	*cardi = deckr.cards - (deckr.playing + deckr.played);

	Deck* deck = deckr.deck;
	uint cards = deckr.cards;

	Card* card;
	uint lim = (cards - (decktop - stacktop)) - 1;

	// look for the last top card, put it on top again
	for (uint i = (cards-1); i > lim; i--) {
		card = &index (deck, i, cards);

		// NOTE: finding a duplicate here actually doensn't matter that much
		if (card->number == _top.number && card->suit == _top.suit) {
			Card* swp = take_from_stack ();
			Card tmp = *swp;

			*swp = *card;
			*card = tmp;

			top = swp;
			break;
		}
	}

	return GOK;
}

Gstat take (Player* p, uint am, bool always) {
	// `alegal' is static because this function receives `always = true'
	// when taking on accumulative or drawing initial cards
	static uint alegal = 0;

	uint pcardi = p->cardi;
	uint* pcards = p->cards;

	Card pcard;
	Card* card;

	uint cardi = deckr.cards - (deckr.playing + deckr.played);

	// skip `E3LEGAL' blocking
	if (always) {
		if (! cardi) {
			Gstat reshufflestat = reshuffle (&cardi, top);
			if (reshufflestat != GOK) {
				return reshufflestat;
			}
		}
		goto takeloop;
	}

	// trigger reshuffle
	else if (! cardi) {
		Gstat reshufflestat = reshuffle (&cardi, top);
		if (reshufflestat != GOK) {
			return reshufflestat;
		}
	}

	// UPDATE (20220908): I decided to make special cards an exception to this rule
	for (uint i = 0; i < pcardi; i++) {
		pcard = index (deckr.deck, pcards[i], cardi);
		if (legal (pcard, *top) && (pcard.suit != SPECIAL)) {
			alegal++;
			if (alegal == 3) {
				alegal = 0;
				EMSGCODE (E3LEGAL);
			}
		}
	}

takeloop:
	for (uint _ = 0; _ < am; _++) {
takecard:
		card = take_from_stack ();
		if ((card && top) && legal (*card, *top)) {
			alegal++;
			
			// NOTE: this allows to get *up-to* three legal cards
			if (alegal == 4) {
				alegal = 0;
				// revert `take_from_stack'
				stacktop--;
				EMSGCODE (E3LEGAL);
			}
		}
		if (!card) {
			Gstat reshufflestat = reshuffle (&cardi, top);
			if (reshufflestat != GOK) {
				return reshufflestat;
			}
			goto takecard;
		}
		MKOWNER (p, card);
		player_append (p, card);
	}

	alegal = 0;
	return GCONT;
}

static inline void psort (Player* p, uint ci, uint pcardi) {
	uint* buf = p->cards;
	uint i = ci;

	for (; i < (pcardi-1); i++) {
		buf[i] = buf[i+1];
	}

	buf[i] = 0;
}

static inline void actioncard (Card* card) {
	switch (card->number)
	{
	case _K: block = true; break;
	case _Q: reverse = true; break;
	case _J: acc += 2; break;
	case _B: acc += 4; break;
	}
}

static inline void playerdrop (Player* p, uint ci) {
	psort (p, ci, p->cardi);
	p->cardi--;
}

void play (Player* p, Card* pc, uint ci) {
	top = pc;

	RMOWNER (pc);
	actioncard (pc);
	playerdrop (p, ci);
}

Player* turn (uint botn) {
	static uint i = 0;

	i += (dir == DIRCLOCKWISE)? 1: (botn-1);
	i %= botn;

	return &playerbuf[i];
}

static void gameinit (Deckr* deckr, uint botn) {
	// init the deck
	uint n = DECKS (botn);
	uint cards = n * CPDECK;
	deckr->n = n;
	deckr->deck = malloc (cards * sizeof (Card));
	deckr->cards = cards;
	deckr->played = 0;

	// arrange the deck
	decktop = &deckr->deck[n-1][CPDECK-1];
	deckbase = &deckr->deck[0][0];
	popdeck (deckr);

	// shuffle the deck
	uint t = time (NULL);
	seed = *(uint*) &botn;
	seed = (seeded (t) << 4) ^ t;
	shuffle (deckr, deckr->cards);
	popplayers (deckr, botn, deckr->cards);

	(void) drawfirstcard ();

	// init the command buffer `cmdbuff'
	cmdbuff = calloc (CMDBUFF, sizeof (char));

#if HEADER == 1
	puts (GAME_HEADER);
#endif
}

typedef struct {
	Gstat master;
	enum {_SLAVE_ACC = 1, } slave;
} GstatUpdate;

static GstatUpdate update_game (Cmd* cmd) {
	Card* pcard;
	GstatUpdate ret = {0};

	// accumulative cards take without asking
	if (cmd->status == CACC) {
		uint _acc = acc;
		acc = 0;
		ret.slave = _SLAVE_ACC;
		ret.master = take (cmd->p, _acc, ALWAYS);
		return ret;
	}

	switch (cmd->ac.cmd) {
	case PLAY:
		pcard = &index (deckr.deck,
										cmd->p->cards[cmd->ac.target-1],
										deckr.cards);

		// ensure suit
		if (pcard->suit == SPECIAL && csuit == NOSUIT) {
			EMSGCODESLAVE (EMISSINGSUIT, ret);
		}

		// ensure legal
		if (! legal (*pcard, *top)) {
			EMSGCODESLAVE (EILLEGAL, ret);
		}

		play (cmd->p, pcard, (cmd->ac.target-1));

		if (block) {
			block = false;
			(void) turn (playern);
		}
		else if (reverse) {
			reverse = false;
			dir = ~dir;
			reverse_draw_players ();
		}

		break;

	case TAKE:
		ret.master = take (cmd->p, cmd->ac.am, CONDITIONAL);
		return ret;
	}

	ret.master = cmd->p->cardi? GCONT: GEND;
	return ret;
}

static int gameloop (uint botn) {
	gameinit (&deckr, botn);

	Cmd cmd;
	GstatUpdate stat;

	cmd.cmdstr = cmdbuff;
	cmd.p = player;

	init_display (botn);

	// if the player doesn't have a legal card, its round gets skipped
	uint i = 0;
getlegalplayer:
	for (; i < botn; i++) {
		for (uint j = 0; j < CPPLAYER; j++) {
			uint* pcards = playerbuf[i].cards;
			Card pcard = index (deckr.deck,
													pcards[j],
													deckr.cards - (deckr.playing + deckr.played));
			// we don't want the first play wasting a special, it's better to skip the turn
			if (legal (pcard, *top) && pcard.suit != SPECIAL)
				goto loop;
		}
		(void) turn (botn);
	}

	if (i == botn) {
		i = 0;
		stacktop--;
		deckr.played--;
		drawfirstcard ();
		goto getlegalplayer;
	}

	goto done;

loop:
	for (;;) {
		if (cmd.p->tag == PLAYER) {
read:
			cmdread (&cmd);
			switch (cmd.status) {
			case CINVALID: goto read;
			case CQUIT: goto done;
			case CHELP: draw_help_msg (&cmd); goto read;
			}
		}
		else {
			bot_play (&cmd, (cmd.p - player));
		}

		stat = update_game (&cmd);

		switch (stat.master) {
		case GMSG_ERR:
			msgsend_err (MSGERRCODE);
			goto read;
		case GMSG_INFO:
			msgsend_info (MSGINFOCODE);
			goto read;
		case GDRAW:
			end_as_draw ();
			goto done;
		case GEND:
			end_as_win (cmd.p);
			goto done;
		}

		update_display (&cmd);

		// only play next turn if the previous turn was played or an accumulative
		// was put in action
		if (cmd.ac.cmd == PLAY || stat.slave == _SLAVE_ACC) {
			cmd.p = turn (botn);
		}
	}

done:
	free (display.buf);
	free (deckr.deck);
	free (playerbuf);
	free (legalbuf);

	return 0;
}

int main (int argc, char** argv) {
	int bots = BOTS;
	playern = (bots+1);

	if (argc > 1) {
		if ((argv[1] && argv[1][0] == '-')) {
			switch (argv[1][1]) {
			case 'v': puts("nou " VERSION); return 1;
			case 'h': default: fputs (
				"Usage: nou [bot number:" BOTSSTR "]\n"
				"Note:  type `q' and press enter to quit at moment\n", stderr); return 1;
			}
		}
		bots = atoi (argv[1]);
		if (bots < 1)
			fputs ("[ WW ] Invalid number, defaulting to " BOTSSTR "\n", stderr);
		else if (bots >= SOFTLIM)
			fputs ("[ WW ] Too many bots, defaulting to " BOTSSTR "\n", stderr);
		else goto game;
		bots = BOTS;
	}

	game: playern = (bots+1);
		
	return gameloop (playern);
}
