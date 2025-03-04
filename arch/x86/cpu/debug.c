#include "screen.h"

void debug(void) {
  viprint("Exception: Debug\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
