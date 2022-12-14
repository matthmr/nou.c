# nou.c - uno™ clone

Card game for the CLI written in C.

## Building

Run `./configure` then `make nou` to build the program. `make`
variables are:

- `CC` : the C compiler (gcc)
- `CFLAGS` : flags to pass to the compiler ()
- `CFLAGSADD`: additional flags to pass to the compiler (after those
  written by `./configure`) ()

`./configure` flags are the same as `make` variables but in
**lowercase** (for example `./configure --cc=clang`) plus the
following include flags:

- `[-/+]header` : ommit/permit drawing game header (permit)

Run `make docs` to compile the man documentation to `docs`.

No further dependencies are needed besides `libc`. This project uses
the `./configure` building system from
[cproj](https://github.com/matthmr/cproj).

## Playing

`nou` plays similar to uno. There are four classes of cards to play:

- numerical cards
- turn cards
- accumulative cards
- anumerical cards

**Numerical cards** are the cards `A` and `2` through `10`. The
general rule of the game is that a player may play a card that is the
same *number* or the same *suit* of the card that was played. So, for
instance, if `2♣` was played, the player may play a `2` of any suit
**or** any card of the same suit as `♣`.

**Turn cards** are the `K` and `Q` cards:

- the `K` card act as a *block card*, it will block the turn of the
  next player.
- the `Q` card act as a *reverse card*, it will reverse the turn of
the game from clockwise to anticlockwise and vice versa. It can also
reverse the attack direction of *accumulative cards*, for instance, if
player A attacks player B with an accumulative, player B can reverse
the attack with a `Q` of the same suit. The reverse mechanic will
reverse the attack to the **last** player whose attack was reversed,
even if it has followed an attack chain.

**Accumulative cards** are the `J` and `B` cards:

- the `J` card will send an attack to the player next to the attacker
turn-wise of taking 2 cards from the mount. Accumulatives can
compound, that is, if player A was attacked by player B with an
accumulative, player A can divert the attack with another
accumulative, regardless of its suit. The compounding of accumulatives
forms an **attack chain** and the value of cards to be taken from the
mount adds up with each accumulative card in the attack chain. So `J♦`
and `J♣` result in 4 cards being taken from the mount.
- the `B` card will send an attack to the player next to the attacker
turn-wise of taking 4 cards from the mount. The attacker must then
choose a suit for the player next the attacked to play. `B` cards can
be chained if the player being attacked has an accumulative of the
suit said by the attacker **or** another `B` card. It can also be
reversed by `Q` if the `Q` in question is of the suit said by the
attacker.  The additive rule applies.

The only **anumerical card** is the `C` card:

- the `C` card may be played in place of any otherwise legal numerical
card. The player must then choose a suit on which the game will
continue with.

The `B` and `C` cards can be played on top of **any suit** if legal to
do so.

If it is a player's turn and they have no legal cards to play, they
must take 1 card from the mount until a legal card appears. If a legal
card appears and the player doesn't want to play it, they can take up
to 2 cards more until they are forced to play it. This also applies if
the player already has a legal card. A player who takes cards from the
mount as the result of an accumulative card will have their turn
skipped.

A **deck** of cards is a set of 54 cards, two of which are the `B` and
`C` card.  As for `K`, `Q`, `J` and the numeric cards (13 cards), they
all have one version *per suit*, with 4 suits in a deck.

For more info run `nou -h`.

This game is *singleplayer only*.

### Interface

Running `nou` by itself throws you into the game prompt. You have the
following commands available:

- '`.`' -- the **dot prefix**
  - it is a command prefix for taking a card, i.e. running `.` by
    itself will take one card from the mount. Combine it with a number
    `<n>` to take *n* cards from the mount (e.g. `.5` takes 5 cards).
- '`,`' -- the **comma prefix**
  - it is a command prefix for playing a card. It needs the card ID as
    the numeric argument (e.g. `,5` plays the 5th card).
- '` `' -- the **space prefix**
  - it is a command prefix for playing the **only** legal move. If
    there's more than one legal move, it will throw an error.
- '`:`' -- the **colon prefix**
  - it is a command prefix for ignoring counteracts. Basically, if you
    are being attacked by a *accumulative card* and have a card that
    can counteract the accumulative, the colon prefix will ignore that
    and do the accumulative action.

After typing your command press ENTER to validate it. If it is not a
legal command, the game will throw an error message so you can type
again. Hitting ENTER on an empty prompt repeats the previous command.

You can also play a card using an unique identifier relating to number
or suit. For example, if you have only two cards, a 2 of spades and a
3 of hearts, to play the 3 of hearts you could type either:

- `3`
- `h`
- `3h`

And that card would be played.

Suits map to characters like: [s]pades, [c]lubs, [h]earts, [d]iamonds.
Specials map to characters like: [C], [B].

If the card you are playing is a *special card*, then the suit must
the typed after the playing command. e.g. `Ch` plays a C card setting
the suit to [h]earts.

## License

This repository is licensed under the [MIT License](https://opensource.org/licenses/MIT).
