## tools 
AS = i686-elf-as
CC = i686-elf-gcc
CFLAGS = -I kernel/include -I drivers/screen/include -I drivers/keyboard/include -I ../../lib/include/ -I cpu/include -I cornucopia/include -ffreestanding -O2 -Wall -Wextra -nostartfiles -nostdlib 
LINKFLAGS = -nostdlib -z max-page-size=0x1000


## main dirs 
bootDir = boot
kernelDir = kernel
srcDir = src
buildDir = build
driversDir = drivers
libDir = lib
cpuDir = cpu 

## file finder and converts
cFiles = $(shell find . -name "*.c")
OBJ = $(patsubst %.c, $(buildDir)/%.o, $(cFiles))

## make the build dirs
$(shell mkdir -p $(buildDir) $(buildDir)/$(bootDir) $(buildDir)/$(kernelDir) $(buildDir)/$(driversDir)/keyboard $(buildDir)/$(libDir) $(buildDir)/$(driversDir)/screen $(buildDir)/$(cpuDir))

## boot 
bootAsm = $(bootDir)/bootloader.s
bootO = $(buildDir)/$(bootDir)/bootloader.o

## main bin and iso file 
osBin = $(buildDir)/cobraOS.bin
isoDir = iso
isoFile = cobra-x86-0.0.2.iso

## transferred lib obj from main dir
libO = $(buildDir)/lib/*.o

## linker 
linkerLd = $(srcDir)/linker.ld

## target
all: $(isoFile)

## asm boot
$(bootO): $(bootAsm)
	@echo "Assembling $(bootAsm)"
	$(AS) $< -o $@

## compile all .c
$(buildDir)/%.o: %.c
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

## link
$(osBin): $(OBJ) $(bootO) $(libO)
	@echo "Linking $(osBin)"
	$(CC) -T $(linkerLd) $(LINKFLAGS) -o $(osBin) $(bootO) $(OBJ) $(libO)

## create required dirs for iso placement 
$(isoFile): $(osBin)
	@echo "Creating ISO $(isoFile)"
	mkdir -p $(isoDir)/boot/grub
	cp $(osBin) $(isoDir)/boot/kernel.bin
	echo 'set timeout=5' > $(isoDir)/boot/grub/grub.cfg
	echo 'set default=0' > $(isoDir)/boot/grub/grub.cfg
	echo 'menuentry "Cobra OS" {' > $(isoDir)/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin' >> $(isoDir)/boot/grub/grub.cfg
	echo '  echo Thats one small step for man, one giant leap for mankind' >> $(isoDir)/boot/grub/grub.cfg
	echo '  boot' >> $(isoDir)/boot/grub/grub.cfg
	echo '}' >> $(isoDir)/boot/grub/grub.cfg
	grub-mkrescue -o $(isoFile) $(isoDir)

## clean
clean:
	rm -rf $(buildDir)/* $(isoDir)/* $(isoFile)
