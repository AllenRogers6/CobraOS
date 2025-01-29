#include "screen.h"

void gpf(void) {
  writeStrToScreen("Exception: General protection fault\n");
  while (1) {
    asm volatile("hlt");
  }
}
