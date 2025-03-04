#include "screen.h"

void bound_range_exceeded(void) {
  viprint("Exception: Bound Range Exceeded\n");

  viprint("Reseting\n");
  asm volatile("jmp 0xFFFF0000");
}
