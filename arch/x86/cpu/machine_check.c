#include "screen.h"

void machine_check(void) {
  viprint("Exception: Machine check\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
