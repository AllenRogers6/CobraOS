#include "screen.h"

void machineCheck(void) {
  writeStrToScreen("Exception: Machine check\n");
  while (1) {
    asm volatile("hlt");
  }
}
