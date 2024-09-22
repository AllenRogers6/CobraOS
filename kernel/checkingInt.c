#include "include/terminal.h"
#include "stdint.h"

int are_interrupts_enabled() {
  uint32_t eflags;
  asm volatile("pushf\n\t"
               "pop %0"
               : "=r"(eflags));

  return eflags & 0x200;
}

void check_interrupts() {
  if (are_interrupts_enabled()) {
    terminal_writeSpace();
    terminal_writestring("IntOn");
  } else {
    terminal_writeSpace();
    terminal_writestring("intOff");
  }
}
