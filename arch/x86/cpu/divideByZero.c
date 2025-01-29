#include "screen.h"

void divideByZero(void) {
  writeStrToScreen("Exception: Division by zero\n");
  while (1) {
    asm volatile("hlt");
  }
}
