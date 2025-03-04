#include "screen.h"
#include "stdint.h"

int are_interrupts_enabled() {
  uint64_t eflags;
  asm volatile("pushf\n\t"
               "pop %0"
               : "=r"(eflags));

  return eflags & 0x200;
}

void check_ints() {
  if (are_interrupts_enabled()) {
    has_loaded();
    viprint("IntOn\n");
  } else {
    has_loaded();
    viprint("intOff\n");
  }
}
