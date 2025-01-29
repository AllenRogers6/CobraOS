#include "screen.h"

void controlProtection(void) {
  writeStrToScreen("Exception: Control protection\n");
  while (1) {
    asm volatile("hlt");
  }
}
