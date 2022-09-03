#ifndef LOCK_BOTS
#  define LOCK_BOTS

#  include "utils.h"
#  include "deck.h"

typedef struct {
	Suit strong_suit, weak_suit;
	Number strong_number, weak_number;

	Suit* strong_oth_suit, * weak_oth_suit;
	Number* strong_oth_number, * weak_oth_number;

	float risk_bias;
} Bot;

typedef enum { PLAYER, BOT, } Ptag;

typedef struct player {
	uint* cards;
	uint cardn, cardi; // @cardn: number of cards in the @cards buffer
	                   // @cardi: number of owned cards
	Ptag tag;          // @tag: player/bot tag
	Bot* bot;          // @bote: bot instance
} Player;

typedef enum {
	COK=0,

	CINVALID,
	CQUIT,
	CHELP,
} CmdStat;

enum cmd {
	TAKE,
	PLAY,

	NOCMD = -1,
};

typedef struct {
	enum cmd cmd;
	uint am;
	uint target; // Card* target;
	//Suit csuit;
} CmdAction;

typedef struct {
	char* cmdstr;
	Player* p;
	CmdStat status;
	CmdAction ac;
} Cmd;

extern char* cmdbuf;

extern Player* playerbuf, * player;
extern uint playern;

void bot_play (Cmd*);
void init_bot (float);

#endif
