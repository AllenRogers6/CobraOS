#include "screen.h"

void virtualization(void) {
  writeStrToScreen("Exception: Virtualization\n");
  while (1) {
    asm volatile("hlt");
  }
}
