#include "screen.h"

void security(void) {
  writeStrToScreen("Exception: Security\n");
  while (1) {
    asm volatile("hlt");
  }
}
