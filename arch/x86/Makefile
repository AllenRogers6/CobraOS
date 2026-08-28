## tools
AS = as
CC = gcc
NASM = nasm
CFLAGS = -I kernel/include -I drivers/screen/include -I drivers/keyboard/include -I drivers/cmos/include \
         -I ../../lib/include -I cpu/include -I cornucopia/include -I fs/include -I snake/include \
         -ffreestanding -O0 -Wall -Wextra -nostartfiles -nostdlib -m32 -g \
         -fno-omit-frame-pointer -mgeneral-regs-only -fno-stack-protector -fno-pie -g
LINKFLAGS = -ffreestanding -nostdlib -no-pie -z max-page-size=0x1000

## directories (relative to arch/x86)
bootDir = boot
cpuDir = cpu
driversDir = drivers
kernelDir = kernel
libDir = ../../lib
srcDir = src
buildDir = build
isoDir = iso
fsDir = fs
snakeDir = snake

## ------------------------------------------------------------
## EXPLICIT SOURCE LISTS 
## ------------------------------------------------------------
c_sources = \
    $(cpuDir)/apic.c \
    $(cpuDir)/checking_int.c \
    $(cpuDir)/gdt.c \
    $(cpuDir)/get_eip.c \
    $(cpuDir)/interrupts.c \
    $(cpuDir)/ldt.c \
    $(cpuDir)/paging.c \
    $(cpuDir)/pic.c \
    $(cpuDir)/pit.c \
    $(cpuDir)/ps2.c \
    $(cpuDir)/serial.c \
    $(cpuDir)/tss.c \
    $(cpuDir)/vmm.c \
    $(driversDir)/screen/screen.c \
    $(driversDir)/keyboard/keyboard.c \
    $(driversDir)/cmos/rtc.c \
    $(kernelDir)/kernel.c \
    $(kernelDir)/task.c \
    $(kernelDir)/stack_protector.c \
    $(kernelDir)/shell.c \
    $(kernelDir)/multiboot.c \
    $(kernelDir)/pmm.c \
    $(kernelDir)/panic.c \
    $(kernelDir)/klog.c \
    $(kernelDir)/heap.c \
    $(kernelDir)/spinlock.c \
		$(fsDir)/ramfs.c \
    $(libDir)/io.c \
    $(libDir)/string.c \
		$(snakeDir)/snake.c \
    $(libDir)/stdio.c


asm_sources = \
    $(cpuDir)/irq_stubs.asm \
    $(cpuDir)/exception_stubs.asm \
    $(cpuDir)/task_switch.asm

boot_src = $(bootDir)/bootloader.s
boot_obj = $(buildDir)/bootloader.o

## flatten objects: all .o files go directly into build/
c_objects   = $(patsubst %.c, $(buildDir)/%.o, $(notdir $(c_sources)))
asm_objects = $(patsubst %.asm, $(buildDir)/%.o, $(notdir $(asm_sources)))

OBJ = $(c_objects) $(asm_objects)

## final outputs
osBin = $(buildDir)/cobraOS.bin
isoFile = cobra-x86-0.0.2.iso
linkerLd = $(srcDir)/linker.ld

## target
all: $(isoFile)

## create build dir
$(shell mkdir -p $(buildDir) $(isoDir))

## assemble bootloader
$(boot_obj): $(boot_src)
	@echo "Assembling bootloader"
	$(AS) --32 $< -o $@

## compile C files (vpath finds sources in their original dirs)
vpath %.c $(sort $(dir $(c_sources)))
$(buildDir)/%.o: %.c
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -c $< -o $@

## assemble NASM files
vpath %.asm $(sort $(dir $(asm_sources)))
$(buildDir)/%.o: %.asm
	@echo "Assembling $<"
	$(NASM) -f elf32 $< -o $@

$(buildDir)/kernel.elf: $(boot_obj) $(OBJ)
	@echo "Linking ELF"
	$(CC) -m32 -T $(linkerLd) $(LINKFLAGS) -o $@ $^

## link
$(osBin): $(boot_obj) $(OBJ)
	@echo "Linking ELF"
	$(CC) -m32 -T $(linkerLd) $(LINKFLAGS) -o build/kernel.elf $^
	@echo "Creating flat binary"
	objcopy -O binary build/kernel.elf $@

## create ISO
$(isoFile): $(buildDir)/kernel.elf
	@echo "Creating ISO $@"
	mkdir -p $(isoDir)/boot/grub
	cp $(buildDir)/kernel.elf $(isoDir)/boot/kernel.elf
	printf 'set timeout=5\nset default=0\nmenuentry "Cobra OS" {\n  multiboot /boot/kernel.elf\n  boot\n}\n' > $(isoDir)/boot/grub/grub.cfg
	grub-mkrescue --output=$@ $(isoDir)

## clean
clean:
	rm -rf $(buildDir) $(isoDir) $(isoFile)

.PHONY: all clean
