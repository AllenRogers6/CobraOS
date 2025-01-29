#include "screen.h"

void segNotPresent(void) {
  writeStrToScreen("Exception: Segment not prsent\n");
  while (1) {
    asm volatile("hlt");
  }
}
