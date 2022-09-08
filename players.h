#ifndef LOCK_PLAYERS
#  define LOCK_PLAYERS

#  include "utils.h"

typedef enum { PLAYER, BOT, } Ptag;

// UPDATE (20220908): `deck.h' was causing some problems at include-time,
// so I made `Player::tag' be the only thing that differentiates a bot
// from a player. I also use `playerbuf' to differentiate between bots
typedef struct player {
	uint* cards;       // @cards: deck index buffer for owned cards
	uint cardn, cardi; // @cardn: size of the @cards buffer
	                   // @cardi: number of owned cards
	Ptag tag;          // @tag: player/bot tag
} Player;

#  include "deck.h"

typedef enum {
	COK=0,

	CINVALID,
	CQUIT,
	CHELP,
	CACC,
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

typedef struct {
 	Suit strong_suit, weak_suit;
 	Number strong_number, weak_number;
 
 	Suit* strong_oth_suit, * weak_oth_suit;
 	Number* strong_oth_number, * weak_oth_number;
 
 	float risk_bias;
} Bot;

extern char* cmdbuf;

extern Player* playerbuf, * player;
extern Bot* botbuf;
extern uint playern;

void bot_play (Cmd*, uint);
void init_bot (float, uint);

#endif
