#include "screen.h"

void tripleFault(void) {
  writeStrToScreen("Exception: Triple fault\n");
  while (1) {
    asm volatile("hlt");
  }
}
