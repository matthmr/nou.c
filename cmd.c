#include <unistd.h>

#include "cmd.h"
#include "draw.h"

const char* errmsg[] = {
	[EINVALIDCMD] = "Invalid command `%s'",
	[ENOSUCHCARDNUM] = "No such playable matching number `%d'",
	[ENOSUCHCARDSUIT] = "No such playable matching suit `%d'",
	[EMULCARDNUM] = "Multiple matches for number `%d' were found",
	[EMULCARDSUIT] = "Multiple matches for suit `%s' were found",
	[ENOSUCHCARDID] = "No such playable matching id `%d'",
	[EMISSINGCMD] = "Missing command. Should be `p' for play or `t' for take",
	[EMISSINGSUIT] = "Missing suit for special card `%s'",
	[EILLEGAL] = "Command plays with illegal card",
	[ETAKE] = "Wrong `t' syntax. Should be: <n>t",
	[EPLAY] = "Wrong `p' syntax. Should be: <n>p or p<num> or p<suit> or p<num><suit>",
	[ECHOOSE] = "Wrong `choose' sytnax. Should be a suit",
};

const char* infomsg[] = {
	[IFOUNDCARD] = "A legal card was found while running the `t' command at iteration %d",
	//[ISUITS] = "spades: `s', clubs: `c', hearts: `h', diamonds: `d'",
};

const char fullmsg[] = \
"\nThe game is similar to UNO. The rules can be thoroughly read in `nou.1' or at the "
"project's `README.md'.\n\n"
"You have two commands: (`t' : take) and (`p' : play)\n\n"
"  - take: the `t' syntax is `<n>t' where `<n>' is an amount of cards to take. If empty, it will\n"
"          be assumed to be 1.\n"
"  - play: the `p' syntax is `<n>p' or `p<num>' or `p<suit>' or `p<num><suit>' where `<n>' is the\n"
"          id of the card to play (the id of the card is the number which prefixes the card of the\n"
"          P0 player), `<num>' is an unique number field of a card in a player's deck and `<suit>'\n"
"          is an unique suit field of a card in the player's deck. If the card to be played is a\n"
"          special card, the suit to be asked comes AFTER the `p' command. For example: `pSh'\n"
"          [p]lays a [S]pecial card then asks for a card in the suit of [h]earts.\n"
"\nHitting enter on an empty prompt repeats the last command.\n\n";

const char cmd[] = { 'p', 't', 'h' };

void help (void) {
	write (1, fullmsg, sizeof (fullmsg));
}

static bool exist_with (Number num, Suit suit) {

}

#define ITER(x,y) for (; (x) < (y); (x)++)
static CmdStat cmdparse (Cmd* cmd, enum err* ecode) {
	struct promise {
		bool _num, _suit, _special, _specialsuit;
		bool __10, __number;
		bool __PAD__[2];
	};

	char* cmdbuf = cmd->cmdstr;

	char cmdnumbuf[MAXCARDSLEN];
	char c, _cmd;

	uint i = 0;
	uint d = 0;
	uint am = 0;

	struct promise promise = {0};

	// look for the easy ones first
	if (*(short*)cmdbuf == CMDHELP) return CHELP;
	if (*(short*)cmdbuf == CMDQUIT) return CQUIT;

	// look for prefixing <n>
	if (cmdbuf[0] == '0') goto invalid_prefix;
	ITER (i, CMDBUFF) {
		c = cmdbuf[i];
		if (NUMBER (c)) {
			promise.__number = 1; // lock it
			if (d <= MAXCARDSLEN) {
				d++; // bound it
			}
			else invalid_prefix: return CINVALID;
		}
		else break;
	}

	// look for the actual command character
	if (c == 'p' || c == 't') {
		cmd->ac.cmd = (c == 'p')? PLAY: TAKE;
		_cmd = c;
		i++;
	}
	else return CINVALID; // we REALLY need one

	// anything other than <n><cmd> errors, eg: 2t2, 9pSh
	if (promise.__number && cmdbuf[i] != '\0') return CINVALID;
	else if (promise.__number) return COK;

	// TODO: ecode, cmd
	// look for `play'-specific shenanigans
	ITER (i, CMDBUFF) {
		c = cmdbuf[i];
		// <suit>
		if (SUIT (c)) {
			if (promise._special) { // get the special card chosen suit
				MAYBE (promise._specialsuit, CINVALID);
			}
			else {
				if (promise._specialsuit) return CINVALID;
				MAYBE (promise._suit, CINVALID);
			}
		}
		// <special>
		if (c == 'S') {
			if (promise._suit | promise._num) return CINVALID;
			MAYBE (promise._special, CINVALID);
		}
		// <num>
		if (NUMBER (c) || c == 'A') {
			// this is straight up a '0'
			if (c == '0' && ! promise.__10) return CINVALID;
			// base 10 fix-up
			if (c == '1') {
				MAYBE (promise.__10, CINVALID); // it still works because 11 is not a card
			}
			// otherwise a number number
			else {
				if (promise.__10) {
					if (c != '0') return CINVALID; // same spiel as before, 11 is not a card
					else {
						promise.__10 = 0;
						goto maybe_num;
					}
				}
				maybe_num: MAYBE (promise._num, CINVALID);
			}
		}
		return CINVALID;
	}

	return COK; // make the compiler happy :)
}

void cmdread (Cmd* cmd) {
	static bool did_error = false;

	enum err ecode;
	CmdStat pstat;
	char* cmdbuf = cmd->cmdstr;

	parse:
	read (0, cmdbuf, CMDBUFF); // TODO buasdj
	pstat = cmdparse (cmd, &ecode);

	// something went wrong
	if (ecode != -1u) {
		error_display (errmsg[ecode]);
		goto parse;
	}
}
