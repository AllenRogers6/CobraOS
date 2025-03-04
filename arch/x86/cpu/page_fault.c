#include "screen.h"
#include "stdint.h"

void page_fault_exception(uint64_t errorCode) {
  uint64_t faultingAddress;
  asm volatile("mov %%cr2, %0" : "=r"(faultingAddress));

  viprint("Exception: page fault\n");
  viprint("Faulting Address: ");
  hexprint(faultingAddress);
  viprint("\nError Code: ");
  hexprint(errorCode);
  viprint("\n");

  while (1) {
    asm volatile("cli; hlt");
  }
}
