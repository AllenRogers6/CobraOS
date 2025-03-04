#include "pit.h"
#include "io.h"
#include "registers.h"
#include "screen.h"
#include <stdint.h>

volatile uint32_t tick = 0;

void pit_handler(registers_t *regs) { tick++; }

void pit_init(uint32_t frequency) {
  uint32_t divisor = PIT_FREQUENCY / frequency;

  outb(0x43, 0x36);

  outb(0x40, (uint8_t)(divisor & 0xFF));
  outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}

void sleep(uint32_t milliseconds) {
  uint32_t target_tick = tick + (milliseconds / 10);
  while (tick < target_tick)
    ;
}
