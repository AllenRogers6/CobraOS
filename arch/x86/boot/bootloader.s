.set MAGIC,    0x1BADB002
.set FLAGS,    0x00000003
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot, "a", @progbits
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

.section .text
.global _start
_start:
    mov $stack_top, %esp

    push %ebx               # multiboot_info_ptr
    push %eax               # magic
    call kernel

    cli
1:  hlt
    jmp 1b

.section .bss
.align 16
stack_bottom:
.skip 32768
stack_top:
