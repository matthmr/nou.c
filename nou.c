#include <unistd.h>
#include <stdlib.h>
#include <time.h>
//#include <stdio.h>

#include "cli.h"
#include "draw.h"
#include "nou.h"

char* cmdbuff = NULL;

bool legal (Card playing, Card played) {
	Suit suit = played.suit, psuit = playing.suit;
	Number num = played.number, pnum = playing.number;

	if (num == _C) {
		return (psuit == csuit || psuit == SPECIAL);
	}
	else if (num == _B) {
		return (
			(psuit == csuit && (pnum == _Q || pnum == _J)) ||
			(psuit == SPECIAL && pnum == _B));
	}
	else if (num == _J) {
		return (
			(psuit == suit && pnum == _Q) ||
			(pnum == _J) ||
			(psuit == SPECIAL && pnum == _B));
	}

	return (pnum == num || psuit == suit || psuit == SPECIAL);
}

bool playable (Player* p, Card played) {
	Deck* deck = deckr.deck;
	uint cardn = deckr.cards;

	uint n = p->cardn;
	uint* cards = p->cards;

	for (uint i = 0; i < n; i++)
		if (legal (index (deck, cards[i], cardn), played)) return true;

	return false;
}

static inline Card* take_card (void) {
	//static Card* takr = NULL;
	if (! stacktop) stacktop = card0;
	else stacktop++;
	if (stacktop == decktop) return stacktop = NULL;
	else return stacktop;
}

static inline void append (Player* p, Card* card) {
	if (p->cardi == (p->cardn+1)) {
		p->cards = realloc (p->cards, (p->cardn + CARDN) * sizeof (uint));
	}
	p->cards[p->cardi] = (card - card0);
	p->cardi++;
}

Gstat take (Player* p, uint am) {
	Card* card;
	for (uint _ = 0; _ < am; _++) {
		card = take_card ();
		if (!card) {
			if (sort (&deckr) == NOCARDS) return GDRAW;
			shuffle (&deckr, deckr.cards - deckr.playing);
			continue;
		}
		playing (card);
		append (p, card);
	}
	return GCONT;
}

static void psort (Player* p, uint cardi) {
	uint cardn = player->cardn;
	uint* buf = player->cards;

	uint* sbuf = NULL;

	uint i = 0, _i = 0;

	for (; i < cardn; i++) {
		if (! buf[i]) {
			if (!sbuf) {
				_i = i;
				sbuf = &buf[i];
			}
		}
		else if (sbuf) {
			sbuf = NULL;
			*sbuf = buf[i];
			i = _i;
		}
	}
}

static inline void drop (Player* p, uint cardi) {
	p->cardi--;
	psort (p, cardi);
}

void play (Player* p, uint ci) {
	Card* pc = &index (deckr.deck, p->cards[ci+1], deckr.cards);
	top = pc;
	played (pc);
	drop (p, ci);
}

static Ptag turn (uint botn) {
	if (dir == DIRCLOCKWISE) {

	}
	else {
	}
}

static void gameinit (Deckr* deckr, uint botn) { // initiate the game
	uint n = DECKS (botn);
	uint cards = n * CPDECK;
	deckr->n = n;
	deckr->cards = cards;
	deckr->deck = malloc (cards * sizeof (Card));
	deckr->playing = PLAYING (botn);
	deckr->played = 0;
	decktop = &deckr->deck[n-1][CPDECK-1];
	popdeck (deckr);
	uint t = time (NULL);
	seed = *(uint*) &botn;
	seed = (reseedr (t) << 4) ^ t;
	shuffle (deckr, deckr->cards);
	popplayers (deckr, botn, deckr->cards);
	//sort (deckr);
	cmdbuff = calloc (CMDBUFF, sizeof (char));
	uint ccardi;
	draw: ccardi = seeded (deckr->cards - deckr->playing);
	Card* ccard = &index (deckr->deck, ccardi, deckr->cards);
	Number dnum = ccard->number;
	if (ccard->suit == SPECIAL || (
	    dnum == _K || dnum == _Q || dnum == _J
	)) goto draw;
	else {
		Card tmp = *stacktop;
		*stacktop = *ccard;
		*ccard = tmp;
		top = stacktop;
		deckr->played++;
	}
	write (1, MSG (GAME_HEADER));
}

static void cmdread (Cmd* cmd) {
	read (0, cmd->cmdstr, CMDBUFF);
}

static Gstat update_game (Cmd* cmd) {

	switch (cmd->ac.cmd) {
	case PLAY: play (cmd->p, cmd->ac.target); break;
	case TAKE: take (cmd->p, cmd->ac.am); break;
	}

	if (!cmd->p->cardi) return GEND; // player `p' wins!
	else return GCONT;
}

static int gameloop (uint botn) { // main game loop
	gameinit (&deckr, botn);

	Cmd cmd;
	Gstat stat;

	cmd.cmdstr = cmdbuff;

	for (;;) {
		if (turn (botn) == PLAYER) {
			read: cmdread (&cmd);
			switch (cmd.status) {
			case CINVALID: goto read;
			case CQUIT: goto end;
			}
			stat = update_game (&cmd);
			if (stat == GDRAW || stat == GEND) goto end;
		}
		else {
			bot_play (&cmd);
			stat = update_game (&cmd);
			if (stat == GDRAW || stat == GEND) goto end;
		}
		update_display ();
	}

	end:
	free (deckr.deck);
	free (display.dis);
	// wins (cmd->p);

	return 0;
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
		else goto game;
		bots = BOTS;
	}
	playern = (bots++);

	game: return gameloop (bots);
}
