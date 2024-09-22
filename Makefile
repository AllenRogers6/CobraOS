## tools
AS = i686-elf-as
CC = i686-elf-gcc
CFLAGS = -g -std=gnu99 -ffreestanding -O2 -Wall -Wextra
LINKFLAGS = -ffreestanding -O2 -nostdlib 


#########
#
# simplify this later on, for easy viewing. 
#
#########

## main dirs
bootDir = boot
kernelDir = kernel
srcDir = src
buildDir = build

## bootloader
bootAsm = $(bootDir)/bootloader.s
bootO = $(bootDir)/bootloader.o

## os bin -- change later to an iso
osBin = $(buildDir)/cobraOS.bin

## to screen
termC = $(kernelDir)/terminal.c
termO = $(buildDir)/terminal.o

## kernel
kernelC = $(kernelDir)/kernel.c
kernelO = $(buildDir)/kernel.o

## linker
linkerLd = $(srcDir)/linker.ld

## IDT + others
intsC = $(kernelDir)/interrupts.c
intsO = $(buildDir)/interrupts.o

## PIC
picC = $(kernelDir)/pic.c
picO = $(buildDir)/pic.o

## checkInt status (ensures int is on)
checkIntC = $(kernelDir)/checkingInt.c
checkIntO = $(buildDir)/checkingInt.o

## the i/o (outx, inx, io_wait)
ioC = $(kernelDir)/io.c
ioO = $(buildDir)/io.o

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

$(ioO): $(ioC)
	@echo "Compiling $(ioC)"
	@echo ""
	$(CC) -c $(ioC) -o $(ioO) $(CFLAGS)

$(picO): $(picC)
	@echo "Compiling $(picC)"
	@echo ""
	$(CC) -c $(picC) -o $(picO) $(CFLAGS)

$(checkIntO): $(checkIntC)
	@echo "Compiling $(checkIntC)"
	@echo ""
	$(CC) -c $(checkIntC) -o $(checkIntO) $(CFLAGS)

$(osBin): $(linkerLd) $(bootO) $(kernelO) $(termO) $(picO) $(intsO) $(checkIntO) $(ioO)
	@echo "Compiling $(osBin)"
	@echo ""
	$(CC) -T $(linkerLd) -o $(osBin) $(LINKFLAGS) $(bootO) $(kernelO) $(termO) $(intsO) $(picO) $(checkIntO) $(ioO)

clean:
	rm -rf $(buildDir)/*.o $(osBin)
