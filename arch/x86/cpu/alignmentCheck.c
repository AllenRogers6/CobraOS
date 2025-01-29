#include "screen.h"

void alignmentCheck(void) {
  writeStrToScreen("Exception: Alignment check\n");
  while (1) {
    asm volatile("hlt");
  }
}
