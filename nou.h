#ifndef LOCK_NOU
#  define LOCK_NOU

#  include "utils.h"
#  include "deck.h"
#  include "players.h"

#  define VERSION "v0.5.2"
#  define PROG "nou"

#  define EMSGCODE(x) return MSGERRCODE = (x), GMSG_ERR

#  define MKOWNER(p,c)													\
	(c)->owner = (p),															\
		deckr.playing++

#  define RMOWNER(c)														\
	(c)->owner = (NULL),													\
		deckr.playing--,														\
		deckr.played++,															\
		top = (c)

typedef enum {
	GDRAW, GEND, GCONT,
	//GACC,
	GMSG_ERR, GMSG_INFO,

	GOK = -1,
} Gstat;

bool legal (Card, Card);
Player* turn (uint);
void play (Player*, Card*, uint);

#  define ALWAYS true
#  define CONDITIONAL false

Gstat take (Player*, uint, bool);

#endif
