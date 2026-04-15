#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <stdint.h>
void set_idt_entry(int vector, uintptr_t handler_address, uint16_t selector,
                   uint8_t type_attr);
void asm_ints_on();
void asm_ints_off();
void default_handler(int vector);
void print_idt_entry(uint8_t vector);
void load();
void verify_idt();
void read_handler_for_vec0();
void read_handler_for_vec33();
void read_esp();
void cmp_base_lim();
void init_ints();

#endif // !INTERRUPTS_H
