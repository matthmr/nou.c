include(make/makefile.m4)dnl
target(`nou')dnl
target_obj(`nou.o',dnl
`bots.o', `deck.o', `draw.o', `cmd.o')dnl
target_gen
