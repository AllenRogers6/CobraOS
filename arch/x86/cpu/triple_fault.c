#include "screen.h"

void triple_fault(void) {
  viprint("Exception: Triple fault\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
