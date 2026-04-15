## tools
AS = as
CC = gcc
NASM = nasm
CFLAGS = -I kernel/include -I drivers/screen/include -I drivers/keyboard/include \
         -I ../../lib/include -I cpu/include -I cornucopia/include \
         -ffreestanding -O0 -Wall -Wextra -nostartfiles -nostdlib -m32 -g \
         -fno-omit-frame-pointer -mgeneral-regs-only -fno-stack-protector -fno-pie
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

## ------------------------------------------------------------
## EXPLICIT SOURCE LISTS (adjust if you have more files)
## ------------------------------------------------------------
c_sources = \
    $(cpuDir)/alignment_check.c \
    $(cpuDir)/apic.c \
    $(cpuDir)/bound_range_exceeded.c \
    $(cpuDir)/breakpoint.c \
    $(cpuDir)/checking_int.c \
    $(cpuDir)/co_seg_overrun.c \
    $(cpuDir)/control_protection.c \
    $(cpuDir)/debug.c \
    $(cpuDir)/dev_not_found.c \
    $(cpuDir)/divide_by_zero.c \
    $(cpuDir)/double_fault.c \
    $(cpuDir)/floating_point_error.c \
    $(cpuDir)/gdt.c \
    $(cpuDir)/get_eip.c \
    $(cpuDir)/gpf.c \
    $(cpuDir)/hypervisor_injection.c \
    $(cpuDir)/interrupts.c \
    $(cpuDir)/invalid_opcode.c \
    $(cpuDir)/invalid_tss.c \
    $(cpuDir)/ldt.c \
    $(cpuDir)/machine_check.c \
    $(cpuDir)/nmi.c \
    $(cpuDir)/overflow.c \
    $(cpuDir)/page_fault.c \
    $(cpuDir)/paging.c \
    $(cpuDir)/pic.c \
    $(cpuDir)/pit.c \
    $(cpuDir)/ps2.c \
    $(cpuDir)/security.c \
    $(cpuDir)/seg_not_present.c \
    $(cpuDir)/serial.c \
    $(cpuDir)/simd_floating_point.c \
    $(cpuDir)/stack_seg_fault.c \
    $(cpuDir)/triple_fault.c \
    $(cpuDir)/tss.c \
    $(cpuDir)/virtualization.c \
    $(cpuDir)/vmm_comms.c \
    $(driversDir)/screen/screen.c \
    $(driversDir)/keyboard/keyboard.c \
    $(kernelDir)/kernel.c \
    $(kernelDir)/stack_protector.c \
    $(libDir)/io.c \
    $(libDir)/string.c \
    $(libDir)/stdio.c

asm_sources = \
    $(cpuDir)/irq_stubs.asm

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

## link
$(osBin): $(boot_obj) $(OBJ)
	@echo "Linking $@"
	$(CC) -m32 -T $(linkerLd) $(LINKFLAGS) -o $@ $^

## create ISO
$(isoFile): $(osBin)
	@echo "Creating ISO $@"
	mkdir -p $(isoDir)/boot/grub
	cp $(osBin) $(isoDir)/boot/kernel.bin
	printf 'set timeout=5\nset default=0\nmenuentry "Cobra OS" {\n  multiboot /boot/kernel.bin\n  echo Thats one small step for man, one giant leap for mankind\n  boot\n}\n' > $(isoDir)/boot/grub/grub.cfg
	grub-mkrescue -o $@ $(isoDir)

## clean
clean:
	rm -rf $(buildDir) $(isoDir) $(isoFile)

.PHONY: all clean
