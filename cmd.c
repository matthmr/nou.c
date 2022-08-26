#include <unistd.h>

#include "cmd.h"
#include "draw.h"
#include "nou.h"

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
	[EMIXING] = "Wrong `n' syntax. Should be: <n><cmd>. Tried to mix",
	[EMULSUIT] = "Multiple card suits were found for command `%s'",
	[EMULNUM] = "Multiple card numbers were found for command `%s'",
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

void help (void) {
	write (1, fullmsg, sizeof (fullmsg));
}

static bool exist_with (Number num, Suit suit) { // see if player has a card of `num' or `suit'
	return true;
}

static CmdStat cmdparse (Cmd* cmd, enum err* ecode) { // parse the player's command
	struct promise {
		bool _num, _suit, _special;
		bool __10, __number;
		bool __PAD__[3];
	};

	struct cmdid {
		uint id;
		char buf [MAXCARDSLEN];
	};

	char* cmdbuf = cmd->cmdstr;

	uint d = 0, id = 0;

	struct promise promise = {0};
	struct cmdid cmdidbuf;

	static Cmd lastcmd = {0};

	// look for the easy ones first
	short easyones = *(short*)cmdbuf;
	switch (easyones) {
		case CMDHELP: return CHELP;
		case CMDQUIT: return CQUIT;
		case CMDNONE: goto done; // TODO: copy `lastcmd' to `cmd'
	}

	char c = '\0';
	uint i = 0;

	// look for prefixing <n>
	if (cmdbuf[0] == '0') goto invalid_prefix;
	ITER (i, CMDBUFF) {
		c = cmdbuf[i];
		if (NUMBER (c)) {
			promise.__number = 1; // lock it
			if (d <= MAXCARDSLEN) {
				d++; // bound it
			}
			else goto invalid_prefix;
		}
		// test if the player even has said card
		if (promise.__number && cmd->p->cardi != id)
			invalid_prefix: ECODE (ENOSUCHCARDID);
		break;
	}

	// look for the actual command character
	if (c == 'p' || c == 't') {
		cmd->ac.cmd = (c == 'p')? PLAY: TAKE;
		i++;
	}
	else if (!c) ECODE (EMISSINGCMD);
	else ECODE (EINVALIDCMD);

	// anything other than <n><cmd> errors `EMIXING', eg: 2t2, 9pSh
	if (promise.__number && cmdbuf[i] != '\0') ECODE (EMIXING);
	else if (promise.__number) ECODE (EOK);
	promise.__number = 0;

	// look for `p'-specific shenanigans
	ITER (i, CMDBUFF) {
		c = cmdbuf[i];
		if (c == '\n') break; // no need to go that far...

		// <suit>
		if (SUIT (c)) {
			// we get here by having a `1' followed by a suit; <num> doesn't trigger `_num' if `__10'
			// is triggered earlier
			if (promise.__10) {
				promise.__10 = 0;
				MAYBE (promise._num, EMULCARDNUM);
			}
			MAYBE (promise._suit, EMISSINGSUIT);
			continue;
		}

		// <special>
		if (c == 'S') {
			// if we have anything before, we error `EMULCARDSUIT'
			if (promise._suit | promise._num | promise._special) ECODE (EMULCARDSUIT);
			MAYBE (promise._special, ECHOOSE);
			continue;
		}

		// <num>
		if (NUMBER (c) || c == 'A') {
			// we get here by having any number where we should have a suit
			if (promise._special) ECODE (EMISSINGSUIT);
			// no card number is `0'-prefixed
			if (c == '0' && ! promise.__10) ECODE (ENOSUCHCARDNUM);
			// no card has `11` as a number
			else if (c == '1') { MAYBE (promise.__10, ENOSUCHCARDNUM) }
			// otherwise, handle it as a normal number
			else if (promise.__10) {
				// same spiel as before, `11' is not a valid card number
				if (c != '0') ECODE (ENOSUCHCARDNUM);
				else promise.__10 = 0;
				goto maybe_num;
			}
			else maybe_num: MAYBE (promise._num, EMULCARDNUM);
			continue;
		}

		// sanity check...
		ECODE (EINVALIDCMD);
	}

	// fallthrough: even if an error managed to squeeze pass the army of `if's
	if ((promise._num + promise._suit + promise._special) != 2)
		ECODE (EINVALIDCMD);

 // make the compiler happy [even though it's garanteed for this function to have a return value]
	done: return COK;
}

void cmdread (Cmd* cmd) {
	CmdStat pstat;
	char* cmdbuf = cmd->cmdstr;

	enum err ecode;
	Card* pcard = NULL;

	parse: ecode = EOK;
	read (0, cmdbuf, CMDBUFF);
	pstat = cmdparse (cmd, &ecode);

	// something went wrong
	if (ecode != EOK) {
		error_display (errmsg[ecode]);
		goto parse;
	}

	else if (pstat == CQUIT || pstat == CHELP) {
		cmd->status = pstat;
		return; // let `gameloop' handle it
	}

	// of course, after ALL THIS it might still be an illegal card...
	if (! legal (*pcard, *top)) {
		error_display (errmsg[EILLEGAL]);
		goto parse;
	}
}
