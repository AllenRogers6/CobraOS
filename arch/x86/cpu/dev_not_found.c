#include "screen.h"

void dev_not_found(void) {
  viprint("Exception: Device not found\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
