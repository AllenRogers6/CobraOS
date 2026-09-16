ARCH_NAME       := x86

CC              := gcc
LD              := gcc
AS              := as
NASM            := nasm
OBJCOPY         := objcopy

QEMU            := qemu-system-i386
QEMU_FLAGS      := -m 128M -serial stdio -no-reboot -no-shutdown -d int,cpu_reset,guest_errors,pcall
QEMU_KERNEL_ARG := -kernel
QEMU_RUN_ARG := -cdrom
BOOT_PROTOCOL   := multiboot1

KERNEL_ELF      := arch/x86/build/kernel.elf
KERNEL_BIN      := arch/x86/build/cobraOS.bin
ISO_NAME        := cobra-x86-$(VERSION).iso

GDB             := gdb
GDB_ARCH        := i386
