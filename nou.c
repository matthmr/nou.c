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
		takecard: card = take_card ();
		if (!card) {
			if (sort (&deckr) == NOCARDS) return GDRAW;
			shuffle (&deckr, deckr.played); //deckr.cards - deckr.playing);
			goto takecard; // continue;
		}
		playing (card);
		append (p, card);
	}
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
	played (pc);
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

Player* dry_turn (uint botn) {
	Player* ret = turn (botn);
	dir = REVERSE (dir);
	(void) turn (botn);
	dir = REVERSE (dir);
	return ret;
}

static void gameinit (Deckr* deckr, uint botn) { // initiate the game
	// init the deck
	uint n = DECKS (botn);
	uint cards = n * CPDECK;
	deckr->n = n;
	deckr->cards = cards;
	deckr->deck = malloc (cards * sizeof (Card));
	//deckr->playing = PLAYING (botn);
	deckr->played = 0;
	decktop = &deckr->deck[n-1][CPDECK-1];
	popdeck (deckr);

	// shuffle the deck
	uint t = time (NULL);
	seed = *(uint*) &botn;
	seed = (reseedr (t) << 4) ^ t;
	shuffle (deckr, deckr->cards);
	popplayers (deckr, botn, deckr->cards);
	//sort (deckr);

	// init the command buffer `cmdbuff'
	cmdbuff = calloc (CMDBUFF, sizeof (char));
	uint ccardi;

	// draw the initial card
draw:
	ccardi = seeded (deckr->cards - deckr->playing) + deckr->playing;
	Card* ccard = &index (deckr->deck, ccardi, deckr->cards);
	Number dnum = ccard->number;
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

	puts (GAME_HEADER);
}

static Gstat update_game (Cmd* cmd) { // update the game according to `cmd'
	Card* pcard;

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
		
		if (block) {
			block = false;
			(void) turn (playern);
		}
		else if (reverse) {
			reverse = false;
			dir = REVERSE (dir);
			set_draw_players_entry_reverse ();
		}

		play (cmd->p, pcard, (cmd->ac.target-1));

		break;

	case TAKE: // TODO: take info message
		return take (cmd->p, cmd->ac.am);
	}

	// TODO: win screen
	if (!cmd->p->cardi) return GEND; // player `p' wins; end the game
	else return GCONT;
}

// TODO: currently, if for some reason the player doesn't have the first
// top card, the game continues whereas by the rules, the turn should go
// to the next player in line. This is not implemented yet
static int gameloop (uint botn) { // main game loop
	gameinit (&deckr, botn);

	Cmd cmd;
	Gstat stat;

	cmd.cmdstr = cmdbuff;

	init_display (botn);

	cmd.p = player;

	for (;;) {
		if (cmd.p->tag == PLAYER) { // TODO: deprecate `p->tag == PLAYER` in favor of `!p->bot`
			read: cmdread (&cmd);
			switch (cmd.status) {
			case CINVALID: goto read;
			case CQUIT: goto end;
			case CHELP: while (draw_help (&cmd)); goto read;
			}
		}
		else bot_play (&cmd);

		stat = update_game (&cmd);

		switch (stat) {
		case GDRAW: case GEND:
			goto end;
		case GMSG_ERR:
			msgsend_err (MSGERRCODE);
			goto read;
		case GMSG_INFO:
			msgsend_info (MSGINFOCODE);
			goto read;
		}

		cmd.p = turn (botn);
		update_display (cmd.p);
	}

	end:
	free (deckr.deck);
	free (display.deck);
	// wins (cmd.p);

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
