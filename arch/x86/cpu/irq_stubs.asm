
%macro ISR_NOERR 1
global isr%1
isr%1:
    cli
    push 0
    push %1
    jmp isr_common
%endmacro

%macro ISR_ERR 1
global isr%1
isr%1:
    cli
    push %1
    jmp isr_common
%endmacro

%macro TABLE_ENTRY 1
    dd isr%1
%endmacro

section .text

%assign i 0
%rep 256
%if i == 8 || i == 10 || i == 11 || i == 12 || i == 13 || i == 14 || i == 17 || i == 21 || i == 29
    ISR_ERR i
%else
    ISR_NOERR i
%endif
%assign i i+1
%endrep

global isr_common
isr_common:
    pusha                 
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10          ; kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp              ; pointer to struct registers
    extern interrupt_dispatch
    call interrupt_dispatch
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popa

    add esp, 8            ; pop vector + error code
    iret

section .data
global isr_stub_table
isr_stub_table:
%assign i 0
%rep 256
    TABLE_ENTRY i
%assign i i+1
%endrep
