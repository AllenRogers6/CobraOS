#include "screen.h"

void devNotFound(void) {
  writeStrToScreen("Exception: Device not found\n");
  while (1) {
    asm volatile("hlt");
  }
}
