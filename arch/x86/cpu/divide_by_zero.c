#include "screen.h"

void divide_by_zero(void) {
  viprint("Exception: Division by zero\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
