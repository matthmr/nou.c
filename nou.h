#ifndef LOCK_NOU
#  define LOCK_NOU

#  include "utils.h"
#  include "bots.h"
#  include "deck.h"

#  define VERSION "v0.3.0"
#  define PROG "nou"

#  define REVERSE(x) ((x)+1)%2

typedef enum {
	GDRAW, GEND, GCONT,
} Gstat;

extern bool legal (Card, Card);
extern bool playable (Player*, Card);

extern Player* turn (uint);
extern Gstat take (Player*, uint);
extern void play (Player*, uint);

#define playing(c) c->playing = 1; deckr.playing++
#define played(c) c->playing = 0; deckr.playing--; deckr.played++; top = c

#endif
