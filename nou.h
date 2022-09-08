#ifndef LOCK_NOU
#  define LOCK_NOU

#  include "utils.h"
#  include "deck.h"
#  include "players.h"

#  define VERSION "v0.5.0"
#  define PROG "nou"

#  define EMSGCODE(x) return MSGERRCODE = (x), GMSG_ERR

#  define OWNS(p,c)				\
	(c)->owner = (p);			\
	deckr.playing++

#  define PLAYS(c)				\
	(c)->owner = (NULL);			\
	deckr.playing--;			\
	deckr.played++;				\
	top = (c)

typedef enum {
	GDRAW, GEND, GCONT,
	//GACC,
	GMSG_ERR, GMSG_INFO,
} Gstat;

bool legal (Card, Card);
Player* turn (uint);
Gstat take (Player*, uint, bool);
void play (Player*, Card*, uint);

#endif
