#ifndef LOCK_BOTS
#  define LOCK_BOTS

#  include "utils.h"
#  include "deck.h"

typedef struct {
	
} Bot;

typedef enum {
	PLAYER, BOT,
} Ptag;

typedef struct player {
	uint* cards;
	uint cardn, cardi; // @cardn: number of cards in the @cards buffer
	                   // @cardi: number of owned cards
	Ptag tag;          // @tag: player/bot tag
	Bot* bot;          // @bote: bot instance
} Player;

typedef enum {
	COK=0, CINVALID, CQUIT, CHELP,
} CmdStat;

enum cmd {TAKE, PLAY};

typedef struct {
	enum cmd cmd;
	uint am;
	uint target;
	//Card* target;
} CmdAction;

typedef struct {
	char* cmdstr;
	Player* p;
	CmdStat status;
	CmdAction ac;
} Cmd;

extern char* cmdbuf;

extern void bot_play (Cmd*);
extern Player* playerbuf, * player;
extern uint playern;

#endif
