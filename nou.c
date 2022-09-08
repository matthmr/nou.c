#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "draw.h"
#include "cli.h"
#include "nou.h"
#include "cmd.h"

char* cmdbuff = NULL;

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

static inline Card* take_card (void) {
	if (! stacktop) stacktop = card0;
	else stacktop++;
	if (stacktop == decktop) return stacktop = NULL;
	else return stacktop;
}

static inline void append (Player* p, Card* card) {
	// TODO: smart memory cleaning; it's not needed now because
	// even the most leaky allocs are only about ~4KB in size
	if (p->cardi == (p->cardn+1)) {
		p->cards = realloc (p->cards,
				    (p->cardn + CARDN) * sizeof (uint));
	}
	p->cards[p->cardi] = (card - card0);
	p->cardi++;
}

Gstat take (Player* p, uint am, bool always) {
	static uint alegal = 0;

	uint pcardi = p->cardi;
	uint* pcards = p->cards;
	Card pcard;
	Card* card;

	// reuse functionality
	if (always) goto takeloop;

	// the "no more than three legal cards" rule
	// UPDATE: I decided to make special cards an exception to this rule
	for (uint i = 0; i < pcardi; i++) {
		pcard = index (deckr.deck,
			       pcards[i],
			       deckr.cards - (deckr.playing + deckr.played));
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
		card = take_card ();
		// same rule as before
		if ((card && top) && legal (*card, *top)) {
			alegal++;
			if (alegal == 4) {
				alegal = 0;
				EMSGCODE (E3LEGAL);
			}
		}
		if (!card) {
			if (sort (&deckr) == NOCARDS)
				return GDRAW;
			shuffle (&deckr, deckr.played);
			goto takecard;
		}
		OWNS (p, card);
		append (p, card);
	}

	alegal = 0;
	return GCONT;
}

static inline void psort (Player* p, uint ci, uint pcardi) {
	uint* buf = p->cards;
	uint i = ci;

	for (; i < (pcardi-1); i++)
		buf[i] = buf[i+1];

	buf[i] = 0;
}

static inline void action (Card* card) {
	switch (card->number)
	{
	case _K: block = true; break;
	case _Q: reverse = true; break;
	case _J: acc += 2; break;
	case _B: acc += 4; break;
	}
}

static inline void drop (Player* p, uint ci) {
	psort (p, ci, p->cardi);
	p->cardi--;
}

void play (Player* p, Card* pc, uint ci) {
	top = pc;
	PLAYS (pc);
	action (pc);
	drop (p, ci);
}

Player* turn (uint botn) {
	static uint i = 0;
	if (dir == DIRCLOCKWISE) {
		i++;
		i %= botn;
	}
	else {
		if (i == 0) i = (botn-1);
		else i--;
	}
	return &playerbuf[i];
}

static void gameinit (Deckr* deckr, uint botn) {
	// init the deck
	uint n = DECKS (botn);
	uint cards = n * CPDECK;
	deckr->n = n;
	deckr->deck = malloc (cards * sizeof (Card));
	deckr->cards = cards;
	deckr->playing = PLAYING (botn);
	deckr->played = 0;
	decktop = &deckr->deck[n-1][CPDECK-1];
	popdeck (deckr);

	// shuffle the deck
	uint t = time (NULL);
	seed = *(uint*) &botn;
	seed = (reseedr (t) << 4) ^ t;
	shuffle (deckr, deckr->cards);
	popplayers (deckr, botn, deckr->cards);

	// init the command buffer `cmdbuff'
	cmdbuff = calloc (CMDBUFF, sizeof (char));

	uint ccardi;
draw:
	// draw the initial card
	ccardi = seeded (deckr->cards - deckr->playing) + deckr->playing;
	Card* ccard = &index (deckr->deck, ccardi, deckr->cards);
	Number dnum = ccard->number;

	// get an actual legal card
	if (ccard->suit == SPECIAL || (
	    dnum == _K || dnum == _Q || dnum == _J
	)) goto draw;
	else {
		stacktop++;
		Card tmp = *stacktop;
		*stacktop = *ccard;
		*ccard = tmp;
		top = stacktop;
		//stacktop++; top++;
		deckr->played++;
	}

#if HEADER == 1
	puts (GAME_HEADER);
#endif
}

static Gstat update_game (Cmd* cmd) {
	Card* pcard;

	// accumulative cards take without asking
	if (cmd->status == CACC) {
		uint _acc = acc;
		acc = 0;
		return take (cmd->p, _acc, true);
	}

	switch (cmd->ac.cmd) {
	case PLAY:
		pcard = &index (deckr.deck,
				cmd->p->cards[cmd->ac.target-1],
				deckr.cards);

		// ensure suit
		if (pcard->suit == SPECIAL && csuit == NOSUIT)
			EMSGCODE (EMISSINGSUIT);

		// ensure legal
		if (! legal (*pcard, *top)) EMSGCODE (EILLEGAL);

		play (cmd->p, pcard, (cmd->ac.target-1));

		if (block) {
			block = false;
			(void) turn (playern);
		}
		else if (reverse) {
			reverse = false;
			dir = ~dir;
			set_draw_players_entry_reverse ();
		}

		break;

	case TAKE: // TODO: take info message
		return take (cmd->p, cmd->ac.am, false);
	}

	return cmd->p->cardi? GCONT: GEND;
}

static int gameloop (uint botn) {
	gameinit (&deckr, botn);

	Cmd cmd;
	Gstat stat;

	cmd.cmdstr = cmdbuff;
	cmd.p = player;

	init_display (botn);

	// if the player doesn't have a legal card, its round gets skipped
	for (uint i = 0; i < botn; i++) {
		for (uint j = 0; j < CPPLAYER; j++) {
			uint* pcards = playerbuf[i].cards;
			Card pcard = index (deckr.deck,
					    pcards[j],
					    deckr.cards - (deckr.playing + deckr.played));
			// we don't want to the first play wasting a special, it's better to skip the turn
			if (legal (pcard, *top) && pcard.suit != SPECIAL)
				goto loop;
		}
		(void) turn (botn);
	}

	// somehow NO player has a legal card, it's better to restart the game before it begins
	// TODO: make it conviently restart (this will be a hassle though...)
	fputs ("\n\n[ !! ] Randomness error. Please restart the game", stderr);
	goto done;

loop:
	for (;;) {
		if (cmd.p->tag == PLAYER) {
read:
			cmdread (&cmd);
			switch (cmd.status) {
			case CINVALID: goto read;
			case CQUIT: goto done;
			case CHELP: while (draw_help (&cmd)); goto read;
			}
		}
		else bot_play (&cmd, 0);

		stat = update_game (&cmd);

		switch (stat) {
		case GMSG_ERR:
			msgsend_err (MSGERRCODE);
			goto read;
		case GMSG_INFO:
			msgsend_info (MSGINFOCODE);
			goto read;
		case GDRAW:
			draw_display ();
			goto done;
		case GEND:
			win_display (cmd.p);
			goto done;
		}

		cmd.p = turn (botn);
		update_display (cmd.p);
	}

done:
	//free (deckr.deck);
	//free (display.deck);

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
