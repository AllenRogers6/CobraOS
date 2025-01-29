#include "screen.h"

void coSegOverrun(void) {
  writeStrToScreen("Exception: Coprocessor segment overrun\n");
  while (1) {
    asm volatile("hlt");
  }
}
