#ifndef LOCK_NOU
#  define LOCK_NOU

#  include "utils.h"
#  include "bots.h"
#  include "deck.h"

#  define VERSION "v0.2.0"
#  define PROG "nou"

#  define REVERSE(x) ((x)+1)%2

typedef enum {
	GDRAW, GEND, GCONT,
} Gstat;

extern bool legal (Card, Card);
extern bool playable (Player*, Card);

extern Gstat take (Player*, uint);
extern void play (Player*, Card*);

//extern void append (Player*, Card*);

#define playing(c) c->playing = 1; c->played = 0
#define played(c) c->playing = 0; c->played = 1

#endif
