#include <unistd.h>
//#include <string.h>

#include "cmd.h"
#include "draw.h"
#include "nou.h"
#include "bots.h"
#include "utils.h"

const char* errmsg[] = {
	[EINVALIDCMD] = "Invalid command",
	[ENOSUCHCARDNUM] = "No such playable matching number'",
	[ENOSUCHCARDSUIT] = "No such playable matching suit'",
	[EMULCARDNUM] = "Multiple matches for number were found",
	[EMULCARDSUIT] = "Multiple matches for suit were found",
	[ENOSUCHCARDID] = "No such playable matching id",
	[EMISSINGSUIT] =  "Missing suit for special card",
	[EILLEGAL] = "Command plays with illegal card",
	[EMIXING] = "Wrong `n' syntax. Tried to mix",
	[EMULSUIT] = "Multiple card suits were found for command",
	[EMULNUM] = "Multiple card numbers were found for command",
	[ENOPREVCMD] = "No previous command to repeat",
};

const char* infomsg[] = {
	[IFOUNDCARD] = "A legal card was found while running the `t' command at iteration %d",
	//[ISUITS] = "spades: `s', clubs: `c', hearts: `h', diamonds: `d'",
};

const char fullmsg[] = \
"\nThe game is similar to UNO. The rules can be thoroughly read in `nou.1' or at the project's `README.md'\n\n"
"You have two prefixes: (`.' : take) and (`,' : play)\n\n"
"  - `.': the `.' prefix is `.<n>' where `<n>' is an amount of cards to take. If empty, it will\n"
"         be assumed to be 1\n"
"  - `,': the `,' prefix is `,<n>' or' where `<n>' is the id of the card to play (the id of the card\n"
"         the number which prefixes the card of the `P0' player)\n\n"
"You can also play a card by passing in an unique suit or number in your deck. For example, to play a\n"
"2 of hearts you could run `2', `2h' or `h' if any of those fields uniquely identified your card.\n"
"Suits are passed by initial: [s]pades, [c]lubs, [h]earts, [d]iamonds\n\n"
"Playing with a special card requires the suit to be passed after the number of the special card, for\n"
"example: `Ch', plays the [C] card asking for a card of [h]earts. This also applies if the card is\n"
"prefixed by `,' is a special cards\n\n"
"Hitting enter on an empty prompt repeats the last command, if there is one.\n\n"
"(Press ENTER to go back to the game)\n";
const uint fullmsgsize = sizeof (fullmsg);

int MSGERRCODE = EOK, MSGINFOCODE = IOK;

static bool findcard (Player* p, Number num, Suit suit) {
	bool has;
	uint pcardi = p->cardi;
	Card card;

	for (uint i = 0; i < pcardi; i++) {
		card = index(deckr.deck, p->cards[i], deckr.cards);

		if (card.number == num);
	}

	return true;
}

#define _SLAVE_IGNORE 0
typedef struct {
	CmdStat master;
	enum {
		_SLAVE_GOTO_DONE = 1,
		_SLAVE_RETURN,
	} slave;
} CmdStatEasy;

static CmdStatEasy cmdparse_easyones (short easy, Cmd* lastcmd, Cmd* cmd, enum err* ecode) {
	CmdStatEasy ret = {0};
	short __little_endian;

	// NOTE: I'm not sure why `read' inverts the bytes so I just
	// invert them back. If anyone encounters a bug with this,
	// comment out the next 3 lines
	__little_endian = (easy & 0x00ff) << 8;
	easy >>= 8;
	easy |= __little_endian;

	switch (easy) {
	case CMDHELP:
		ret.master = CHELP;
		ret.slave = _SLAVE_RETURN;
		return ret;
	case CMDQUIT:
		ret.master = CQUIT;
		ret.slave = _SLAVE_RETURN;
		return ret;
	case CMDNONE:
		if (lastcmd->ac.cmd == NOCMD) {
			*ecode = ENOPREVCMD;
			ret.master = CINVALID;
			ret.slave = _SLAVE_RETURN;
		}
		else {
			*cmd = *lastcmd;
			ret.master = COK;
			ret.slave = _SLAVE_GOTO_DONE;
		}
		return ret;
	}

}

typedef struct {
	CmdStat master;
	enum { _SLAVE_GOTO_INVALID_PREFIX = 1, } slave;

	uint num;
} CmdStatNum;

static CmdStatNum cmdparse_number (char* buf, enum err* ecode) {
	CmdStatNum ret = {0};

	uint d = 0, id = 0, pos = 1;

	uint i = 0;
	char c;

	// `0' prefix : no bueno
	if (buf[i] == '0') {
		*ecode = ENOSUCHCARDID;
		ret.master = CINVALID;
		ret.slave = _SLAVE_GOTO_INVALID_PREFIX;
		goto done;
	}

	ITER (c, buf, i, CMDBUFF) {
		if (NUMBER (c)) {
			id += ATOI (c)*pos;
			pos *= 10;
			d++;
		}
		else goto done;
	}

done:
	ret.num = id;
	return ret;
}

static CmdStat cmdparse (Cmd* cmd, enum err* ecode) {
	struct promise {
		bool _play_num, _player_suit, _play_special;
		bool _take_am;
		bool __PAD__[4];
	};

	struct promise promise = {0};

	static Cmd lastcmd = {.ac = {.cmd = NOCMD}};

	char* cmdbuf = cmd->cmdstr;

	// look for the easy ones first
	CmdStatEasy easyones = cmdparse_easyones (*(short*) cmdbuf, &lastcmd, cmd, ecode);
	if (easyones.slave != _SLAVE_IGNORE) {
		if (easyones.slave == _SLAVE_GOTO_DONE)
			goto done;
		else if (easyones.slave == _SLAVE_RETURN)
			return easyones.master;
	}

	uint i = 0;
	uint id = 0;
	char c = cmdbuf[i];

	// prefixing <.>: take by amount
	if (cmdbuf[i] == '.') {
		INC (c, cmdbuf, i);

		if (_DONE (c)) {
			cmd->ac.cmd = TAKE;
			cmd->ac.am = 1;
			lastcmd = *cmd;
			goto done;
		}
		else {
			CmdStatNum statnum = cmdparse_number (cmdbuf, ecode);

			if (statnum.slave == _SLAVE_IGNORE) {
				cmd->ac.cmd = TAKE;
				cmd->ac.am = statnum.num;
				lastcmd = *cmd;
			}

			return statnum.master;
		}
	}

	// prefixing <,>: play by card id
	else if (cmdbuf[i] == ',') {
		INC (c, cmdbuf, i);

		if (_DONE (c)) {
			cmd->ac.cmd = PLAY;
			cmd->ac.target = 1;
			lastcmd = *cmd;
			goto done;
		}
		else {
			CmdStatNum statnum = cmdparse_number (cmdbuf, ecode);

			if (statnum.slave == _SLAVE_IGNORE) {
				cmd->ac.cmd = PLAY;
				cmd->ac.target = statnum.num;
				lastcmd = *cmd;
			}

			return statnum.master;
		}
	}

	// prefixing <>: play by card attribute
	else {

	}

	done: return COK;
}

void msgsend_err (enum err ecode) {
	error_display (errmsg[ecode]);
}

void msgsend_info (enum info icode) {
	return;
}

void cmdread (Cmd* cmd) {
	enum err ecode;

	char* cmdbuf = cmd->cmdstr;
	uint _read;

	parse: ecode = EOK;
	_read = read (0, cmdbuf, CMDBUFF);

	// handle C-d (aka '\0' terminated read callbacks)
	if (! _read) goto parse;
	else if (cmdbuf[_read-1] != '\n') fix_display (); // TODO<-

	cmdbuf[_read] = '\0';
	cmd->status = cmdparse (cmd, &ecode);

	// something went wrong: display the error, wait for more input
	if (ecode != EOK) {
		error_display (errmsg[ecode]);
		goto parse;
	}
}
