#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "registers.h"
#include <stdint.h>

typedef void (*irq_handler_t)(struct registers *);
typedef uint32_t (*syscall_handler_t)(uint32_t, uint32_t, uint32_t, uint32_t);

void init_ints(void);
void interrupt_dispatch(struct registers *r);

int irq_register_handler(uint8_t irq, irq_handler_t handler);
void irq_unregister_handler(uint8_t irq);

int syscall_register(uint8_t num, syscall_handler_t handler);

void asm_ints_on(void);
void asm_ints_off(void);
void verify_idt(void);
void read_esp(void);

#endif
