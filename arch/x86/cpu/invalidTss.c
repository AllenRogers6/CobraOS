#include "screen.h"

void invalidTss(void) {
  writeStrToScreen("Exception: Invalid tss\n");
  while (1) {
    asm volatile("hlt");
  }
}
