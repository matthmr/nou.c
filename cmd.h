#ifndef LOCK_CMD
#  define LOCK_CMD

#  include "cli.h"
#  include "players.h"

//#  define CMDHELP 0x3f0a
#  define CMDQUIT 0x710a
#  define CMDNONE 0x0a00

#  define ERRMSGLEN 44
//#  undef ERRMSGLEN

#  ifndef ERRMSGLEN
#    error ERRMSGLEN is not defined yet. Did you run ./configure?
#  endif

#  define ITER(x,y,z,w) for (; (z) < (w); (z)++, (x) = (y)[(z)])

#  define ECODE(x) return *ecode = (x), CINVALID
#  define ECODESLAVE(x,y) return *ecode = (y), (x).master = CINVALID, (x);

#  define MAYBE(x,y) if (! (x)) { (x) = 1; } else { ECODE (y); }
#  define MAYBESLAVE(x,y) if (! (x).slave) { (x).slave = 1; } else { ECODESLAVE (x, y) }

#  define _EOK(x) return *ecode = (x), COK

#  define INC(x,y,z) (z)++; (x) = (y)[(z)]
#  define _DONE(x) ((x) == '\n' || !(x))

#  define _SUIT(x) ((x) == 's' || (x) == 'c' || (x) == 'h' || (x) == 'd')
#  define _SPECIAL(x) ((x) == 'B' || (x) == 'C')
#  define _LETTER(x) (				\
	((x) | ICASE) == 'a' ||			\
	((x) | ICASE) == 'k' ||			\
	((x) | ICASE) == 'q' ||			\
	((x) | ICASE) == 'j')
#  define _NUMBER(x) (_LETTER (x) || NUMBER (x)) // || _SPECIAL (x)

enum err {
	EINVALIDCMD,
	ENOSUCHCARDNUM,
	ENOSUCHCARDSUIT,
	EMULCARDNUM,
	EMULCARDSUIT,
	EMULCARD,
	ENOSUCHCARDID,
	EMISSINGSUIT,
	EILLEGAL,
	EMIXING,
	EMULSUIT,
	EMULNUM,
	ENOPREVCMD,
	ENOLEGAL,
	EMULLEGAL,
	E3LEGAL,

	EOK = -1,
};

enum info {
	IFOUNDCARD,
	//ISUITS,

	IOK = -1,
};

extern const char* errmsg[];
extern const char* infomsg[];

extern int MSGERRCODE, MSGINFOCODE;

void cmdread (Cmd*);
void msgsend_err (enum err);
void msgsend_info (enum info);

#endif
