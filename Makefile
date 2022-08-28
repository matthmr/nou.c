default: nou

include make/Flags.mk
include make/Objects.mk
include make/Targets.mk

MAKEFILES:=make/Flags.mk make/Objects.mk make/Targets.mk
DOCUMENTATION:=docs/nou.1

$(OBJECTS):
	@echo "[ .. ] Compiling \`$<'"
	$(CC) $(CFLAGS) $(CFLAGSADD) -c $< -o $@

$(TARGETS):
	@echo "[ .. ] Linking to \`$@'"
	$(CC) $(CFLAGS) $(CFLAGSADD) $? -o $@

docs/nou.1: docs/nou.md
	@echo "[ .. ] Compiling man documentation"
	$(MD2ROFF) $< > docs/nou.1

clean:
	@echo "[ .. ] Cleaning working directory"
	rm -rfv $(OBJECTS) $(TARGETS)

clean-make:
	@echo "[ .. ] Cleaning makefiles"
	rm -rfv $(MAKEFILES)

clean-docs:
	@echo "[ .. ] Cleaning documentation"
	rm -rfv $(DOCUMENTATION)

clean-tags:
	@echo "[ .. ] Cleaning tags"
	rm -fv tags

tags:
	@echo "[ .. ] Making tags file"
	ctags --output-format=etags -f TAGS -R .

.PHONY: clean clean-make clean-docs clean-tags tags
