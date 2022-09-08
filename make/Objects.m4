deck.o: deck.c nou.h utils.h deck.h players.h
	@echo "CC deck.c"
	@$(CC) -c M4FLAG_include_deck $(CFLAGS) $(CFLAGSADD) $< -o $@
draw.o: draw.c draw.h utils.h players.h deck.h nou.h cli.h cmd.h
	@echo "CC draw.c"
	@$(CC) -c M4FLAG_include_draw $(CFLAGS) $(CFLAGSADD) $< -o $@
cmd.o: cmd.c players.h utils.h deck.h cmd.h cli.h draw.h nou.h
	@echo "CC cmd.c"
	@$(CC) -c M4FLAG_include_cmd $(CFLAGS) $(CFLAGSADD) $< -o $@
nou.o: nou.c draw.h utils.h players.h deck.h cli.h nou.h cmd.h
	@echo "CC nou.c"
	@$(CC) -c M4FLAG_include_nou $(CFLAGS) $(CFLAGSADD) $< -o $@
players.o: players.c players.h utils.h deck.h nou.h
	@echo "CC players.c"
	@$(CC) -c M4FLAG_include_players $(CFLAGS) $(CFLAGSADD) $< -o $@
OBJECTS:=deck.o draw.o cmd.o nou.o players.o 

