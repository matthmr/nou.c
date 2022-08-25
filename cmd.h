#ifndef LOCK_CMD
#  define LOCK_CMD

#  include "cli.h"
#  include "bots.h"

#  define CMDHELP 0x6800
#  define CMDQUIT 0x7100

#  define __errmsgbufsize 70
#  define ERRMSGLEN __errmsgbufsize
//#  undef ERRMSGLEN // we make this trigger #error, then we delete this line

#  ifndef ERRMSGLEN
#    error `_errmsgbufsize' is not defined yet. Did you run ./configure?
#  endif

#  define SUIT(x) ((x) == 's' || (x) == 'c' || (x) == 'h' || (x) == 'd')
#  define MAYBE(x,y) if (! (x)) (x) = 1; else return (y);

enum err {
	EINVALIDCMD,
	ENOSUCHCARDNUM,
	ENOSUCHCARDSUIT,
	EMULCARDNUM,
	EMULCARDSUIT,
	ENOSUCHCARDID,
	EMISSINGCMD,
	EMISSINGSUIT,
	EILLEGAL,

	ETAKE, EPLAY, ECHOOSE,
	EOK = -1,
};

enum info {
	IFOUNDCARD,
	//ISUITS,
};

extern const char* errmsg[];
extern const char* infomsg[];
extern const char fullmsg[];

extern const char cmd[];

void help (void);
void cmdread (Cmd*);

#endif
