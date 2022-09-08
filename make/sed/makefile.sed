s/(.*?)\.o: .*$/\0\n\t@echo "CC \1.c"\n\t@$(CC) -c M4FLAG_include_\1 $(CFLAGS) $(CFLAGSADD) $< -o $@/g
