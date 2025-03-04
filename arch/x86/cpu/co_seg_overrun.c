#include "screen.h"

void co_seg_overrun(void) {
  viprint("Exception: Coprocessor segment overrun\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
