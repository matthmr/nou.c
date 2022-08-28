#ifndef LOCK_CMD
#  define LOCK_CMD

#  include "cli.h"
#  include "bots.h"

#  define CMDHELP 0x6800
#  define CMDQUIT 0x7100
#  define CMDNONE 0x0a00

#  define ERRMSGLEN 70
//#  undef ERRMSGLEN

#  ifndef ERRMSGLEN
#    error `_errmsgbufsize' is not defined yet. Did you run ./configure?
#  endif

#  define SUIT(x) ((x) == 's' || (x) == 'c' || (x) == 'h' || (x) == 'd')
#  define ITER(x,y,z,w) for (; (y) < (w); (y)++, (z) = (x)[(y)])

#  define ECODE(x) return *ecode = (x), CINVALID
#  define MAYBE(x,y) if (! (x)) { (x) = 1; } else { ECODE (y); }

#  define INC(x,y,z) (z)++; (x) = (y)[(z)]
#  define DONE(x) ((x) == '\n' || !(x))

enum err {
	EINVALIDCMD,
	ENOSUCHCARDNUM,
	ENOSUCHCARDSUIT,
	EMULCARDNUM,
	EMULCARDSUIT,
	ENOSUCHCARDID,
	// EMISSINGCMD,
	EMISSINGSUIT,
	EILLEGAL,
	EMIXING,
	EMULSUIT,
	EMULNUM,
	EMISSINGID,

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

// extern const char cmd[];

void help (void);
void cmdread (Cmd*);

#endif
