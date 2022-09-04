#include <unistd.h>
//#include <string.h>

#include "cmd.h"
#include "draw.h"
#include "nou.h"
#include "bots.h"
#include "utils.h"

#define _SLAVE_IGNORE 0

const char* errmsg[] = {
	[EINVALIDCMD] = "Invalid command",
	[ENOSUCHCARDNUM] = "No such playable matching number'",
	[ENOSUCHCARDSUIT] = "No such playable matching suit'",
	[EMULCARDNUM] = "Multiple matches for number were found",
	[EMULCARDSUIT] = "Multiple matches for suit were found",
	[EMULCARD] = "Multiple matches for card were found",
	[ENOSUCHCARDID] = "No such playable matching id",
	[EMISSINGSUIT] =  "Missing suit for special card",
	[EILLEGAL] = "Command plays with illegal card",
	[EMIXING] = "Wrong syntax. Tried to mix",
	[EMULSUIT] = "Multiple card suits were found for command",
	[EMULNUM] = "Multiple card numbers were found for command",
	[ENOPREVCMD] = "No previous command to repeat",
	[ENOLEGAL] = "No legal command found",
	[EMULLEGAL] = "Multiple legal commands found",
};

const char* infomsg[] = {
	[IFOUNDCARD] = "A legal card was found while running the `t' command at iteration %d",
	//[ISUITS] = "spades: `s', clubs: `c', hearts: `h', diamonds: `d'",
};

const char fullmsg[] = \
"\nThe game is similar to UNO. The rules can be thoroughly read in `nou.1' or at the project's `README.md'\n\n"
"You have three prefixes: (`.' : take), (`,' : play), (` ' : legal play)\n\n"
"  - `.': the `.' prefix is `.<n>' where `<n>' is an amount of cards to take. If empty, it will\n"
"         be assumed to be 1\n"
"  - `,': the `,' prefix is `,<n>' or' where `<n>' is the id of the card to play (the id of the card\n"
"         the number which prefixes the card of the `P0' player)\n"
"  - ` ': the ` ' (empty space) prefix is ` '. It will play the only legal move possible, otherwise error\n"
"         if there are more than one legal move to play\n\n"
"You can also play a card by passing in an unique suit or number in your deck. For example, to play a\n"
"2 of hearts you could run `2', `2h' or `h' if any of those fields uniquely identified your card.\n"
"Suits are passed by initial: [s]pades, [c]lubs, [h]earts, [d]iamonds\n\n"
"Playing with a special card requires the suit to be passed after the number of the special card, for\n"
"example: `Ch', plays the [C] card asking for a card of [h]earts. This also applies if the card is\n"
"prefixed by `,' or ` ' is a special card\n\n"
"Hitting enter on an empty prompt repeats the last command, if there is one\n\n"
"(Press ENTER to go back to the game)\n";
const uint fullmsgsize = sizeof (fullmsg);

int MSGERRCODE = EOK, MSGINFOCODE = IOK;

static inline Number findcard_number (char buf[2]) {
	short easy = *(short*) buf;
	if (! easy) return NONUMBER;
	else {
		if (buf[0] == '1' && buf[1] == '0')
			return _10;

		switch (buf[0]) {
		case 'A': return _A;
		case 'K': return _K;
		case 'Q': return _Q;
		case 'J': return _J;
		case 'B': return _B;
		case 'C': return _C;
		}

		return (ATOI (buf[0]) - 1);
	}
}

static inline Suit findcard_suit (char buf) {
	switch (buf) {
	case 's': return SPADES;
	case 'c': return CLUBS;
	case 'h': return HEARTS;
	case 'd': return DIAMONDS;
	default: return NOSUIT;
	}
}

typedef struct {
	CmdStat master;
	bool slave;

	uint target;
} CmdStatFindcard;

static CmdStatFindcard findcard (Player* p,
				 Number num,
				 Suit suit,
				 enum err* ecode) {
	CmdStatFindcard ret = {0};
	uint pcardi = p->cardi;
	Card card;

	// chose the part of the function to execute
	if (num == NONUMBER) goto by_suit;
	else if (suit == NOSUIT) goto by_num;
	
	for (uint i = 0; i < pcardi; i++) {
		card = index (deckr.deck, p->cards[i], deckr.cards);

		if (card.number == num && card.suit == suit) {
			MAYBESLAVE (ret, EMULCARD);
			ret.target = i;
		}
	}

by_num:
	for (uint i = 0; i < pcardi; i++) {
		card = index(deckr.deck, p->cards[i], deckr.cards);

		if (card.number == num) {
			MAYBESLAVE (ret, EMULCARDNUM);
			ret.target = i;
		}
	}

by_suit:
	for (uint i = 0; i < pcardi; i++) {
		card = index(deckr.deck, p->cards[i], deckr.cards);

		if (card.suit == suit) {
			MAYBESLAVE (ret, EMULCARDSUIT);
			ret.target = i;
		}
	}

	return ret;
}

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
	default: return ret;
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
	char c = buf[i];

	// `0' prefix : no bueno
	if (buf[i] == '0') goto invalid_prefix;

	ITER (c, buf, i, CMDBUFF) {
		if (NUMBER (c)) {
			id += ATOI (c)*pos;
			pos *= 10;
			d++;
		}
		else break;

		if (d > MAXCARDSLEN) invalid_prefix: {
			*ecode = ENOSUCHCARDID;
			ret.master = CINVALID;
			ret.slave = _SLAVE_GOTO_INVALID_PREFIX;
			return ret;
		}
	}

	ret.num = id;
	return ret;
}

typedef struct {
	CmdStat master;
	enum { _SLAVE_LEGAL = 1, } slave;

	uint target;
} CmdStatLegal;

static CmdStatLegal cmdparse_legal (Player* p, enum err* ecode) {
	CmdStatLegal ret = {0};
	uint pcardi = p->cardi;
	Card card;

	for (uint i = 0; i < pcardi; i++) {
		card = index (deckr.deck, p->cards[i], deckr.cards);
		if (legal (card, *top)) {
			MAYBESLAVE (ret, EMULLEGAL);
			ret.target = i;
		}
	}

	if (ret.slave == _SLAVE_IGNORE)
		*ecode = ENOLEGAL;

	return ret;
}

static CmdStat cmdparse (Cmd* cmd, enum err* ecode) {
	static Cmd lastcmd = {.ac = {.cmd = NOCMD}};

	struct promise {
		bool _play_num, _play_suit, _play_special;
		bool __10;
		bool __PAD__[4];
	} promise = {0};

	struct val {
		char play_suit;
		char* play_number;
	} val = {0};

	char _val_play_number[2] = {0,0};
	val.play_number = _val_play_number;

	char* cmdbuf = cmd->cmdstr;
	uint i = 0;
	char c = cmdbuf[i];

	// look for the easy ones first
	CmdStatEasy stateasy = cmdparse_easyones (*(short*) cmdbuf,
						  &lastcmd,
						  cmd,
						  ecode);
	if (stateasy.slave != _SLAVE_IGNORE) {
		if (stateasy.slave == _SLAVE_GOTO_DONE)
			goto done;
		else if (stateasy.slave == _SLAVE_RETURN)
			return stateasy.master;
	}

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
			CmdStatNum statnum = cmdparse_number (cmdbuf+1, ecode);

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
			CmdStatNum statnum = cmdparse_number (cmdbuf+1, ecode);

			if (statnum.slave == _SLAVE_IGNORE) {
				cmd->ac.cmd = PLAY;
				cmd->ac.target = statnum.num;
				lastcmd = *cmd;
			}
			if (index (deckr.deck,
				   cmd->p->cards[statnum.num-1],
				   deckr.cards).suit == SPECIAL) {
				promise._play_special = true;
				i++;
				c = cmdbuf[i];
				goto play_by_attr;
			}

			else return statnum.master;
		}
	}

	// prefixing < >: play the only legal move
	else if (cmdbuf[i] == ' ') {
		INC (c, cmdbuf, i);

		CmdStatLegal statlegal = cmdparse_legal (cmd->p, ecode);

		if (statlegal.slave == _SLAVE_LEGAL) {
			cmd->ac.cmd = PLAY;
			cmd->ac.target = statlegal.target;
			lastcmd = *cmd;
		}

		if (index (deckr.deck,
			   cmd->p->cards[statlegal.target],
			   deckr.cards).suit == SPECIAL) {
			promise._play_special = true;
			i++;
			c = cmdbuf[i];
			goto play_by_attr;
		}

		else return statlegal.master;
		
	}

	cmd->ac.cmd = PLAY;

play_by_attr:
	// prefixing <>: play by card attribute
	ITER (c, cmdbuf, i, CMDBUFF) {
		if (_DONE (c)) break;

		if (_SUIT (c)) {
			// `1' followed by suit: `1' is not a card number
			if (promise.__10) ECODE (ENOSUCHCARDNUM);

			MAYBE (promise._play_suit, EMULSUIT);
			val.play_suit = c;
		}

		else if (_NUMBER (c)) {
			// <special> is not followed by a number
			if (promise._play_special) ECODE (EMISSINGSUIT);

			if (c == '0' && ! promise.__10) ECODE (ENOSUCHCARDNUM); // `0'-prefix
			else if (c == '1') {
				MAYBE (promise.__10, ENOSUCHCARDNUM);
				val.play_number[0] = c;
			}
			else if (promise.__10) {
				if (c != '0') ECODE (ENOSUCHCARDNUM);
				else promise.__10 = 0;
				goto maybe_num;
			}
			else {
			maybe_num:
				MAYBE (promise._play_num, EMULCARDNUM);
				*(val.play_number + (val.play_number[0] != '\0')) = c;
			}
			
		}

		else if (_SPECIAL (c)) {
			// <special> precedes all
			if (promise._play_suit | promise._play_num | promise._play_special)
				ECODE (EMULNUM);
			else {
				MAYBE (promise._play_special, EMULSUIT);
				*(val.play_number + (val.play_number[0] != '\0')) = c;
			}
		}

		else ECODE (EINVALIDCMD);
	}

	if (promise._play_special && ! promise._play_suit) ECODE (EMISSINGSUIT);

	CmdStatFindcard statfindcard = findcard (cmd->p,
						 findcard_number (val.play_number),
						 (promise._play_special?
						  (csuit = findcard_suit (val.play_suit), NOSUIT):
						   findcard_suit (val.play_suit)),
						  ecode);

	if (statfindcard.slave == _SLAVE_IGNORE) {
		cmd->ac.target = statfindcard.target;
		goto done;
	}
	else return statfindcard.master;

done:
	return COK;
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

parse:
	ecode = EOK;
	_read = read (0, cmdbuf, CMDBUFF);

	// handle C-d (aka '\0' terminated read callbacks)
	if (! _read) goto parse;
	else if (cmdbuf[_read-1] != '\n') fix_display (); // TODO<-

	cmdbuf[_read] = '\0';
	cmd->status = cmdparse (cmd, &ecode);

	// something went wrong
	if (ecode != EOK) {
		error_display (errmsg[ecode]);
		// NOTE: the line below could be removed. I just kept them
		// to spare a function call :)
		goto parse;
	}
}
