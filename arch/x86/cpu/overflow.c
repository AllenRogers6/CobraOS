#include "screen.h"

void overflow(void) {
  viprint("Exception: Overflow\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
