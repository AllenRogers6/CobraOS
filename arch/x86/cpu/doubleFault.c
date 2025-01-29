#include "screen.h"

void doubleFaultException() {
  writeStrToScreen("Exception: double fault");
  while (1) {
    asm volatile("hlt");
  }
}
// doubleFaultHandler
