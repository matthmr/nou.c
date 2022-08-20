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
#  define SOFTLIM 1000
#  define CMDBUFF 50
#  define THINK_INTERVAL 500*1000 // 500 ms
#  define TAKE_INTERVAL 500*1000 // 100 ms

#  define ONES(x) ~(~(1<<(x))+1)

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned char bool;

#  ifndef NULL
#    define NULL ((void*)0)
#  endif

#endif