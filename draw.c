#include <stdlib.h>
#include <stdio.h>
//#include <unistd.h>

#include "draw.h"
#include "deck.h"
#include "nou.h"
#include "cli.h"
#include "cmd.h"

#define REDCARD __ESC__ "91m"
#define BLACKCARD __ESC__ "90m"
#define SPECIALCARD __ESC__ "1m"

Display display;

CardCell ccell;
PlayerCardCell pccell;

const schar numpm[] = {
	[_A] = "A",
	[_2] = "2",
	[_3] = "3",
	[_4] = "4",
	[_5] = "5",
	[_6] = "6",
	[_7] = "7",
	[_8] = "8",
	[_9] = "9",
	[_10] = "10",
	[_K] = "K",
	[_Q] = "Q",
	[_J] = "J",
	[_B] = "B",
	[_C] = "C",
};

const wchar suitpm[] = {
	[DIAMONDS] = "♦",
	[CLUBS] = "♣",
	[HEARTS] = "♥",
	[SPADES] = "♠",

	[HOLLOW (DIAMONDS)] = "♢",
	[HOLLOW (CLUBS)] = "♧",
	[HOLLOW (HEARTS)] = "♡",
	[HOLLOW (SPADES)] = "♤",
};

static inline ansi* color_card (Card* card) {
	if (card->suit == HEARTS || card->suit == DIAMONDS)
		return (ansi*)& (REDCARD);
	else if (card->suit == CLUBS || card->suit == SPADES)
		return (ansi*)& (BLACKCARD);
	else
		return (ansi*)& (SPECIALCARD);
}

static void highlight_player (Player* p) {
	if (p->tag == BOT) {
		
	}
	else {
		
	}
}

static void draw_command_prompt (char* buf) {

}

static void draw_deck (char* buf) {

}

static void draw_player_entry (char* buf) {

}

static void draw_bot_entry (char* buf) {

}

void error_display (const char* msg) {

}

void init_display (uint botn) {
	uint pcardn = player->cardn;

	display.prompt = calloc (PROMPTLEN + ERRMSGLEN + 2, sizeof (char));
	display.player = calloc ((pcardn+1), sizeof (PlayerCardCell)); // we assume one more card to fit the player prompt
	display.deck = calloc (DECKENTRYLEN + 1, sizeof (char));
	display.bots = calloc (BOTENTRYLEN * botn, sizeof(char));

	draw_command_prompt (display.prompt);
	draw_deck (display.deck);
	draw_player_entry (display.player);
	draw_bot_entry (display.bots);
}

void update_display (Cmd* cmd) {
	static uint high = 0;
}
