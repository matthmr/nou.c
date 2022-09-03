#ifndef LOCK_CMD
#  define LOCK_CMD

#  include "cli.h"
#  include "bots.h"

#  define CMDHELP 0x3f0a
#  define CMDQUIT 0x710a
#  define CMDNONE 0x0a0a

#  define ERRMSGLEN 70
//#  undef ERRMSGLEN

#  ifndef ERRMSGLEN
#    error `ERRMSGLEN' is not defined yet. Did you run ./configure?
#  endif

#  define ITER(x,y,z,w) for (; (z) < (w); (z)++, (x) = (y)[(z)])

#  define ECODE(x) return *ecode = (x), CINVALID
#  define _EOK(x) return *ecode = (x), COK
#  define MAYBE(x,y) if (! (x)) { (x) = 1; } else { ECODE (y); }

#  define INC(x,y,z) (z)++; (x) = (y)[(z)]
#  define _DONE(x) ((x) == '\n' || !(x))

#  define _SUIT(x) ((x) == 's' || (x) == 'c' || (x) == 'h' || (x) == 'd')
#  define _SPECIAL(x) ((x) == 'B' || (x) == 'C')
#  define _NUMBER(x) ((x) == 'A' || ((x) >= '2' && (x) <= '9'))

enum err {
	EINVALIDCMD,
	ENOSUCHCARDNUM,
	ENOSUCHCARDSUIT,
	EMULCARDNUM,
	EMULCARDSUIT,
	ENOSUCHCARDID,
	EMISSINGSUIT,
	EILLEGAL,
	EMIXING,
	EMULSUIT,
	EMULNUM,
	ENOPREVCMD,

	EOK = -1,
};

enum info {
	IFOUNDCARD,
	//ISUITS,

	IOK = -1,
};

extern const char* errmsg[];
extern const char* infomsg[];
extern const char fullmsg[];
extern const uint fullmsgsize;

extern int MSGERRCODE, MSGINFOCODE;

void cmdread (Cmd*);
void msgsend_err (enum err);
void msgsend_info (enum info);

#endif
