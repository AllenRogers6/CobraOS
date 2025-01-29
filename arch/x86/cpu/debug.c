#include "screen.h"

void debug(void) {
  writeStrToScreen("Exception: Debug\n");
  while (1) {
    asm volatile("hlt");
  }
}
