#include "screen.h"

void overflow(void) {
  writeStrToScreen("Exception: Overflow\n");
  while (1) {
    asm volatile("hlt");
  }
}
