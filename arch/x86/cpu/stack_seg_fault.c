#include "screen.h"

void stack_seg_fault(void) {
  viprint("Exception: Stack segment fault\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
