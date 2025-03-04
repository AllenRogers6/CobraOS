#include "screen.h"

void hypervisor_injection(void) {
  viprint("Exception: Hypervisor injection\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
