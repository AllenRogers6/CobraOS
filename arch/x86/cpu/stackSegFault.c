#include "screen.h"

void stackSegFault(void) {
  writeStrToScreen("Exception: Stack segment fault\n");
  while (1) {
    asm volatile("hlt");
  }
}
