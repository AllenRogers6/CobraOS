#include "screen.h"

void invalidOpcode(void) {
  writeStrToScreen("Exception: Invalid opcode\n");
  while (1) {
    asm volatile("hlt");
  }
}
