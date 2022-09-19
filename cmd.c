#include <stdlib.h>
#include <unistd.h>
//#include <string.h>

#include "players.h"
#include "cmd.h"
#include "draw.h"
#include "nou.h"
#include "utils.h"

#define _SLAVE_IGNORE 0

const char* errmsg[] = {
	[EINVALIDCMD] = "Invalid command",
	[ENOSUCHCARDNUM] = "No such playable matching number",
	[ENOSUCHCARDSUIT] = "No such playable matching suit",
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
	[E3LEGAL] = "More than 3 legal cards were found",
};

const char* infomsg[] = {
	[IFOUNDCARD] = "A legal card was found while running the `t'",
	//[ISUITS] = "spades: `s', clubs: `c', hearts: `h', diamonds: `d'",
};

const char fullmsg[] = \
	"\nThe game is similar to UNO. The rules can be thoroughly read in `nou.1' or at the project's `README.md'\n\n"
	"You have four prefixes: (`.' : take), (`,' : play), (` ' : legal play) & (`:' : ignore legal for accumulative)\n\n"
	"  - `.': the `.' prefix is `.<n>' where `<n>' is an amount of cards to take. If empty, it will\n"
	"         be assumed to be 1\n"
	"  - `,': the `,' prefix is `,<n>' or' where `<n>' is the id of the card to play (the id of the card\n"
	"         the number which prefixes the card of the `P0' player)\n"
	"  - ` ': the ` ' (empty space) prefix is ` '. It will play the only legal move possible, otherwise error\n"
	"         if there are more than one legal move to play\n"
	"  - `:': the `:' prefix is `:'. It will ignore legal counteracts for accumulative cards and resolute their\n"
	"         actions\n\n"
	"You can also play a card by passing in an unique suit or number in your deck. For example, to play a\n"
	"2 of hearts you could run `2', `2h' or `h' if any of those fields uniquely identified your card.\n"
	"Suits are passed by initial: [s]pades, [c]lubs, [h]earts, [d]iamonds\n\n"
	"Playing with a special card requires the suit to be passed after the number of the special card, for\n"
	"example: `Ch', plays the [C] card asking for a card of [h]earts. This also applies if the card is\n"
	"prefixed by `,' or ` ' is a special card\n\n"
	"Hitting enter on an empty prompt repeats the last command, if there is one\n\n"
	"(Press ENTER to go back to the game) ";
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
			ret.target = (i+1);
		}
	}
	if (! ret.slave) ECODESLAVE (ret, EILLEGAL);
	goto done;

by_num:
	for (uint i = 0; i < pcardi; i++) {
		card = index(deckr.deck, p->cards[i], deckr.cards);

		if (card.number == num) {
			MAYBESLAVE (ret, EMULCARDNUM); 
			ret.target = (i+1);
		}
	}
	if (! ret.slave) ECODESLAVE (ret, ENOSUCHCARDNUM);
	goto done;

by_suit:
	for (uint i = 0; i < pcardi; i++) {
		card = index(deckr.deck, p->cards[i], deckr.cards);

		if (card.suit == suit) {
			MAYBESLAVE (ret, EMULCARDSUIT);
			ret.target = (i+1);
		}
	}
	if (! ret.slave) ECODESLAVE (ret, ENOSUCHCARDSUIT);

done:
	return ret;
}

typedef struct {
	CmdStat master;
	enum {
		_SLAVE_GOTO_DONE = 1,	_SLAVE_RETURN
	} slave;
} CmdStatEasy;

static CmdStatEasy cmdparse_easyones (char* buf, Cmd* lastcmd, Cmd* cmd, enum err* ecode) {
	short easy = *(short*)buf;
	CmdStatEasy ret = {0};

	// NOTE: I'm not sure why `read' inverts the bytes so I just
	// invert them back. If anyone encounters a bug with this,
	// comment out the next 3 lines
	short __little_endian = (easy & 0x00ff) << 8;
	easy >>= 8;
	easy |= __little_endian;

	switch (easy) {
	case CMDHELP:
		ret.master = CHELP;
		ret.slave = _SLAVE_RETURN;
		break;
	case CMDQUIT:
		ret.master = CQUIT;
		ret.slave = _SLAVE_RETURN;
		break;
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
		break;
	}

	return ret;
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

	// `0' prefix
	if (buf[i] == '0') {
		goto invalid_prefix;
	}

	// dirty hack for `pos'
	INC (c, buf, i);

	// find out how many digits we have
	ITER (c, buf, i, CMDBUFF) {
		if (NUMBER (c)) {
			pos *= 10, d++;
		}
		else {
			break;
		}
	}

	if (d > MAXCARDSLEN) {
invalid_prefix:
		*ecode = ENOSUCHCARDID;
		ret.master = CINVALID;
		ret.slave = _SLAVE_GOTO_INVALID_PREFIX;
		return ret;
	}

	for (uint i = 0; (i <= d) && (pos); i++) {
		c = buf[i];
		id += ATOI (c)*pos;
		pos /= 10;
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
			ret.target = (i+1);
		}
	}

	if (ret.slave == _SLAVE_IGNORE)
		*ecode = ENOLEGAL;

	return ret;
}

static CmdStatLegal cmdparse_legalacc (Player* p, enum err* ecode) {
	CmdStatLegal ret = {0};
	uint pcardi = p->cardi;
	Card card;

	for (uint i = 0; i < pcardi; i++) {
		card = index (deckr.deck, p->cards[i], deckr.cards);
		// give the player a change to counteract
		if (legal (card, *top)) {
			ret.master = COK;
			ret.slave = _SLAVE_LEGAL;
			goto done;
		}
	}

	ret.master = CACC;
	ret.slave = _SLAVE_IGNORE;

done:
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
	CmdStatEasy stateasy = cmdparse_easyones (cmdbuf,	&lastcmd, cmd, ecode);

	if (stateasy.slave != _SLAVE_IGNORE) {
		switch (stateasy.slave) {
		case _SLAVE_GOTO_DONE:
			goto done;
		case _SLAVE_RETURN:
			return stateasy.master;
		}
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

			// clear overshot
			if (statnum.num > (deckr.cards - (deckr.played + deckr.playing))) {
				ECODE (EINVALIDCMD);
			}

			if (statnum.slave == _SLAVE_IGNORE) {
				cmd->ac.cmd = TAKE;
				cmd->ac.am = statnum.num;
				lastcmd = *cmd;
			}

			return statnum.master;
		}
	}

	// prefixing <:>: ignore otherwise legal plays and do accumulative action
	else if (cmdbuf[i] == ':') {
		INC (c, cmdbuf, i);

		if (! _DONE (c) || ! acc) {
			ECODE (EINVALIDCMD);
		}
		else {
			// NOTE: so that `:' still updates the display correclty
			return CACCDRAW;
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

			// clear overshot
			if (statnum.num > cmd->p->cardi)
				ECODE (EINVALIDCMD);

			if (statnum.slave == _SLAVE_IGNORE) {
				cmd->ac.cmd = PLAY;
				cmd->ac.target = statnum.num;
				lastcmd = *cmd;
			}
			else goto statnum_master;

			Card card = index (deckr.deck,
					   cmd->p->cards[statnum.num-1],
					   deckr.cards);

			if (card.suit == SPECIAL) {
				c = numpm[card.number][0];
				goto _special;
			}
			else statnum_master: return statnum.master;
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
		else goto statlegal_master;
		Card card = index (deckr.deck,
				   cmd->p->cards[statlegal.target],
				   deckr.cards);

		if (card.suit == SPECIAL) {
			c = numpm[card.number][0];
			goto _special;
		}
		else statlegal_master: return statlegal.master;
		
	}

	cmd->ac.cmd = PLAY;

	// prefixing <>: play by card attribute
	ITER (c, cmdbuf, i, CMDBUFF) {
		if (_DONE (c)) {
			break;
		}

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

		else if (_SPECIAL (c)) _special: {
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

	if (statfindcard.slave != _SLAVE_IGNORE) {
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
	return; // TODO: <-
}

void cmdread (Cmd* cmd) {
	enum err ecode;

	char* cmdbuf = cmd->cmdstr;
	uint _read;

	// send accumalative callback only if there are no cards available to be played
	if (acc) {
		CmdStatLegal statlegalacc = cmdparse_legalacc (cmd->p, &ecode);
		if (statlegalacc.slave == _SLAVE_IGNORE) {
			// TODO: send an info message
			cmd->status = statlegalacc.master;
			return;
		}
	}

parse:
	ecode = EOK;
	_read = read (0, cmdbuf, CMDBUFF);

	// handle C-d (aka '\0' terminated read callbacks)
	if (! _read) {
		goto parse;
	}
	else if (cmdbuf[_read-1] != '\n') {
		fix_display (); // TODO: <-
	}

	cmdbuf[_read] = '\0';
	cmd->status = cmdparse (cmd, &ecode);

	// something went wrong
	if (ecode != EOK) {
		error_display (errmsg[ecode]);
		goto parse;
	}
}
