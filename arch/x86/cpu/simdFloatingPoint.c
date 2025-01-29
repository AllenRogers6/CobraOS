#include "screen.h"

void simdFloatingPoint(void) {
  writeStrToScreen("Exception: SIMD floating point\n");
  while (1) {
    asm volatile("hlt");
  }
}
