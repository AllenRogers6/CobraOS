#include "screen.h"

void hypervisorInjection(void) {
  writeStrToScreen("Exception: Hypervisor injection\n");
  while (1) {
    asm volatile("hlt");
  }
}
