#include "screen.h"

void nmi(void) {
  writeStrToScreen("Exception: Non-maskable interrupt\n");
  while (1) {
    asm volatile("hlt");
  }
}
