#include "screen.h"

void nmi(void) {
  viprint("Exception: Non-maskable interrupt\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
