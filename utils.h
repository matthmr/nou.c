#ifndef LOCK_UTILS
#  define LOCK_UTILS

#  define MSG(x) x, sizeof ((x))

#  define false 0
#  define true 1

#  define CPDECK 54
#  define CPPLAYER 7
#  define DECKS(p) ((((p)-1)/3)+1)
#  define MOUNT(p) ((DECKS (p)*CPDECK)-CPPLAYER*p)
#  define PLAYING(p) (CPPLAYER*p)

#  define BOTS 2
#  define BOTSSTR "2"
#  define SOFTLIM 100
#  define THINK_INTERVAL 500*1000 // 500 ms
#  define TAKE_INTERVAL 500*1000 // 100 ms


typedef unsigned int uint;

#endif
