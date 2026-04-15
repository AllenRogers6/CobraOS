all:
	$(MAKE) -C arch/x86

clean:
	$(MAKE) -C arch/x86 clean

.PHONY: all clean
