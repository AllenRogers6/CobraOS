#include "screen.h"

void simd_floating_point(void) {
  viprint("Exception: SIMD floating point\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
