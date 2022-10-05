default: nou

-include make/Flags.mk
-include make/Objects.mk
-include make/Targets.mk

MAKEFILES:=make/Flags.mk make/Objects.mk make/Targets.mk make/Autodep.mk make/Objects.m4

#$(OBJECTS):
#	@echo "CC " $<
#	@$(CC) $(CFLAGS) $(CFLAGSADD) -c $< -o $@

$(TARGETS):
	@echo "CC -o" $@
	@$(CC) $(CFLAGS) $(CFLAGSADD) $(OBJECTS) -o $@

clean:
	@echo "RM " $(OBJECTS) $(TARGETS)
	@rm -rfv $(OBJECTS) $(TARGETS)

clean-make:
	@echo "RM " $(MAKEFILES)
	@rm -rfv $(MAKEFILES)

clean-tags:
	@echo "RM TAGS"
	@rm -fv TAGS

tags:
	@echo "CTAGS -f TAGS"
	@ctags --output-format=etags -f TAGS -R .

.PHONY: clean clean-make clean-tags tags
