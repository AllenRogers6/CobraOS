#include "screen.h"

void breakpoint(void) {
  writeStrToScreen("Exception: Breakpoint\n");
  while (1) {
    asm volatile("hlt");
  }
}
