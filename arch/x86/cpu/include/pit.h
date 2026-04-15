#ifndef PIT_H
#define PIT_H

#include "registers.h"
#include <stdint.h>

#define PIT_FREQUENCY 1193182

void pit_init(uint32_t frequency);
void pit_handler(registers_t *regs);
void sleep(uint32_t milliseconds);

#endif
