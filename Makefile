## tools
AS = i686-elf-as
CC = i686-elf-gcc
CFLAGS = -I kernel/include -g -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LINKFLAGS = -ffreestanding -O2 -nostdlib 

## main dirs
bootDir = boot
kernelDir = kernel
srcDir = src
buildDir = build
driversDir = drivers

cFiles = $(shell find . -name "*.c")
OBJ = $(patsubst %.c, $(buildDir)/%.o, $(cFiles))

$(shell mkdir -p $(buildDir) $(buildDir)/$(bootDir) $(buildDir)/$(kernelDir) $(buildDir)/$(driversDir)/keyboard)

## bootloader
bootAsm = $(bootDir)/bootloader.s
bootO = $(buildDir)/bootloader.o

## os bin -- change later to an iso
osBin = $(buildDir)/cobraOS.bin

## linker
linkerLd = $(srcDir)/linker.ld

all: $(osBin)

$(bootO): $(bootAsm)
	@echo "Assembling  $(bootAsm)"
	@echo ""
	$(AS) -o $@ $<

$(buildDir)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(osBin): $(OBJ) $(bootO)
	@echo "Compiling $(osBin)"
	@echo ""
	$(CC) -T $(linkerLd) $(LINKFLAGS) -o $(osBin) $(OBJ) $(bootO)

clean:
	rm -rf $(buildDir)/*
