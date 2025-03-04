#include "screen.h"

void virtualization(void) {
  viprint("Exception: Virtualization\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
