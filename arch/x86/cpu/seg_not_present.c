#include "screen.h"

void seg_not_present(void) {
  viprint("Exception: Segment not prsent\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
