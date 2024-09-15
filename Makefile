AS = i686-elf-as
CC = i686-elf-gcc
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LINKFLAGS = -ffreestanding -O2 -nostdlib 

## main dirs
bootDir = boot
kernelDir = kernel
libcDir = libc
srcDir = src
buildDir = build

## asm
bootAsm = $(bootDir)/bootloader.s
bootO = $(bootDir)/bootloader.o

## os
osBin = $(buildDir)/cobraOS.bin

## terminal
termC = $(kernelDir)/terminal.c
termO = $(buildDir)/terminal.o

## kernel
kernelC = $(kernelDir)/kernel.c
kernelO = $(buildDir)/kernel.o

## linker
linkerLd = $(srcDir)/linker.ld

## IDT
intsC = $(kernelDir)/interrupts.c
intsO = $(buildDir)/interrupts.o

## PIC
picC = $(kernelDir)/pic.c
picO = $(buildDir)/pic.o

## checkInt
checkIntC = $(kernelDir)/checkingInt.c
checkIntO = $(buildDir)/checkingInt.o


all: $(osBin)

$(bootO): $(bootAsm)
	@echo "Assembling  $(bootAsm)"
	@echo ""
	$(AS) $(bootAsm) -o $(bootO)

$(kernelO): $(kernelC)
	@echo "Compiling $(kernelC)"
	@echo ""
	$(CC) -c $(kernelC) -o $(kernelO) $(CFLAGS)

$(termO): $(termC)
	@echo "Compiling $(termC)"
	@echo ""
	$(CC) -c $(termC) -o $(termO) $(CFLAGS)

$(intsO): $(intsC)
	@echo "Compiling $(intsC)"
	@echo ""
	$(CC) -c $(intsC) -o $(intsO) $(CFLAGS)

$(picO): $(picC)
	@echo "Compiling $(picC)"
	@echo ""
	$(CC) -c $(picC) -o $(picO) $(CFLAGS)


$(checkIntO): $(checkIntC)
	@echo "Compiling $(checkIntC)"
	@echo ""
	$(CC) -c $(checkIntC) -o $(checkIntO) $(CFLAGS)

$(osBin): $(linkerLd) $(bootO) $(kernelO) $(termO) $(picO) $(intsO) $(checkIntO)
	@echo "Compiling $(osBin)"
	@echo ""
	$(CC) -T $(linkerLd) -o $(osBin) $(LINKFLAGS) $(bootO) $(kernelO) $(termO) $(intsO) $(picO) $(checkIntO)

clean:
	rm -rf $(osBin) $(bootO) $(kernelO) $(termO) $(intsO) $(picO) $(checkIntO)
