#include "screen.h"

void security(void) {
  viprint("Exception: Security\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
