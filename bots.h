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
	uint* cards, cardn, cardi;
	Ptag tag;
	Bot* bot;
} Player;

typedef enum {
	CINVALID, CQUIT,
} CmdStat;

enum cmd {TAKE, PLAY};

typedef struct {
	enum cmd cmd;
	uint am;
	Card* target;
} CmdAction;

typedef struct {
	char* cmdstr; // t1, t4, p1h, 2p, pqc, ...
	Player* p;
	CmdStat status;
	CmdAction ac;
} Cmd;

extern char* cmdbuf;

extern void bot_play (Cmd*);
extern Player* playerbuf, * player;
extern uint playern;

#endif
