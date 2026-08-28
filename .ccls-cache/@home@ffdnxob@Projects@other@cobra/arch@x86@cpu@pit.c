#include "pit.h"
#include "io.h"
#include "screen.h"
#include "task.h"

volatile uint32_t tick_count = 0;

void pit_init(uint32_t frequency) {
  uint32_t divisor = 1193182 / frequency;
  outb(0x43, 0x36); // Command: channel 0, lobyte/hibyte, rate generator
  outb(0x40, divisor & 0xFF);        // Low byte
  outb(0x40, (divisor >> 8) & 0xFF); // High byte
}

void pit_handler() {
  tick_count++;

  // Print a dot every ~1 second (100 ticks at 100 Hz)
  if (tick_count % 100 == 0) {
    // Use direct VGA write to avoid complex screen calls in ISR
    uint16_t *vga = (uint16_t *)0xB8000;
    static int pos = 0;
    vga[pos] = (0x0F00 | '.');
    pos = (pos + 1) % (80 * 25);
  }
  schedule();
  outb(0x20, 0x20); // Send EOI
}
