#include "screen.h"

void vmmComms(void) {
  writeStrToScreen("Exception: VMM communication\n");
  while (1) {
    asm volatile("hlt");
  }
}
