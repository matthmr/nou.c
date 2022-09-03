#ifndef LOCK_NOU
#  define LOCK_NOU

#  include "utils.h"
#  include "bots.h"
#  include "deck.h"

#  define VERSION "v0.4.2"
#  define PROG "nou"

#  define REVERSE(x) ((x)+1)%2

#  define EMSGCODE(x) return MSGERRCODE = (x), GMSG_ERR

typedef enum {
	GDRAW, GEND, GCONT,

	GMSG_ERR, GMSG_INFO,
} Gstat;

extern bool legal (Card, Card);
extern bool playable (Player*, Card);

extern Player* turn (uint);
extern Player* dry_turn (uint);

extern Gstat take (Player*, uint);
extern void play (Player*, Card*, uint);

#define playing(c) c->playing = 1; deckr.playing++
#define played(c) c->playing = 0; deckr.playing--; deckr.played++; top = c

#endif
