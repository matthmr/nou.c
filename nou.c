#include <unistd.h>
#include <stdlib.h>
#include <time.h>
//#include <stdio.h>

#include "cli.h"
#include "deck.h"
#include "bots.h"

static bool legal (Card playing, Card played) {
	Suit suit = played.suit, psuit = playing.suit;
	Number num = played.number, pnum = playing.number;

	if (num == _C) {
		return (psuit == csuit || psuit == SPECIAL);
	}
	else if (num == _B) {
		return ((psuit == csuit && pnum == _J) || (psuit == SPECIAL && pnum == _B));
	}
	else if (num == _J) {
		return ((psuit == suit && pnum == _Q) || (pnum == _J) || (psuit == SPECIAL && pnum == _B));
	}

	return (pnum == num || psuit == suit || psuit == SPECIAL);
}

static void gameinit (Deckr* deckr, uint botn) { // initiate the game
	uint n = DECKS (botn+1);
	uint cards = n * CPDECK;
	deckr->n = n;
	deckr->cards = cards;
	deck = malloc (cards * sizeof (Card));
	deckr->deck = deck;
	deckr->playing = PLAYING (botn+1);
	deckr->played = 0;
	popdeck (deckr);
	uint t = time (NULL);
	seed = *(uint*) &botn;
	seed = (reseedr (t) << 4) ^ t;
	shuffle (deckr, deckr->cards);
	popbots (deckr, botn);
	sort (deckr);
	write (1, MSG (GAME_HEADER));
}

static int gameloop (uint botn) { // main game loop
	Deckr deckr = {
		.deck = deck,
		.n = 0,
		.cards = 0,
		.playing = 0,
		.played = 0
	};
	gameinit (&deckr, botn);

	for (;;) {
		read: if (read () == RINVALID) goto read;
		switch (update ()) {
		case UQUIT: goto end; break;
		case UILEGAL: goto read; break;
		}
		render ();
	}

	end: free (deckr.deck);
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

	game: return gameloop (bots);
}
