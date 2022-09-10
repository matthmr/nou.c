default: nou

-include make/Flags.mk
-include make/Objects.mk
-include make/Targets.mk

MAKEFILES:=make/Flags.mk make/Objects.mk make/Targets.mk make/Autodep.mk make/Objects.m4
DOCUMENTATION:=docs/nou.1

#$(OBJECTS):
#	@echo "CC " $<
#	@$(CC) $(CFLAGS) $(CFLAGSADD) -c $< -o $@

$(TARGETS):
	@echo "CC -o" $@
	@$(CC) $(CFLAGS) $(CFLAGSADD) $(OBJECTS) -o $@

docs/nou.1: docs/nou.md
	@echo "[ .. ] Compiling man documentation"
	$(MD2ROFF) $< > docs/nou.1

clean:
	@echo "RM " $(OBJECTS) $(TARGETS)
	@rm -rfv $(OBJECTS) $(TARGETS)

clean-make:
	@echo "RM " $(MAKEFILES)
	@rm -rfv $(MAKEFILES)

clean-docs:
	@echo "RM " $(DOCUMENTATION)
	@rm -rfv $(DOCUMENTATION)

clean-tags:
	@echo "RM TAGS"
	@rm -fv TAGS

tags:
	@echo "CTAGS -f TAGS"
	@ctags --output-format=etags -f TAGS -R .

.PHONY: clean clean-make clean-docs clean-tags tags
