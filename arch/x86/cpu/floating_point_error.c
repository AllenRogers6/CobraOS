#include "screen.h"

void floating_point_error(void) {
  viprint("Exception: Floating point error\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
