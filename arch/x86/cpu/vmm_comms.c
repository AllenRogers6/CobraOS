#include "screen.h"

void vmm_comms(void) {
  viprint("Exception: VMM communication\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
