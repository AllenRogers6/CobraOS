// #include "get_eip.h"
#include "registers.h"
#include "screen.h"
#include "stdio.h"

void double_fault_exception() {
  uint16_t *vga = (uint16_t *)0xB8000;
  vga[0] = (0x0F00 | 'D');
  vga[1] = (0x0F00 | 'F');
  while (1) {
  }
}
