#include "screen.h"
#include <stdint.h>

unsigned long read_cr2() {
  unsigned long cr2_value;
  __asm__ volatile("mov %%cr2, %0" : "=r"(cr2_value));
  return cr2_value;
}

void enable_alignment_checking() {
  uintptr_t eflags;
  __asm__ volatile("pushf; pop %0" : "=r"(eflags));
  eflags |= 0x40000;
  __asm__ volatile("push %0; popf" : : "r"(eflags));
}

void alignment_check(void) {
  viprint("Exception: Alignment check\n");
  while (1) {
    asm volatile("cli; hlt");
  }
}
