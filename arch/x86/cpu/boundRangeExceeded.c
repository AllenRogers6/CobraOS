#include "screen.h"

void boundRangeExceeded(void) {
  writeStrToScreen("Exception: Bound Range Exceeded\n");
  while (1) {
    asm volatile("hlt");
  }
}
