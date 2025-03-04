#include "control_protection.h"
#include "screen.h"

void control_protection(void) {
  viprint("Exception: Control protection\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
