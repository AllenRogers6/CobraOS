#include "screen.h"

void breakpoint(void) {
  viprint("Exception: Breakpoint\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
