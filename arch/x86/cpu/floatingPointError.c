#include "screen.h"

void floatingPointError(void) {
  writeStrToScreen("Exception: Floating point error\n");
  while (1) {
    asm volatile("hlt");
  }
}
