#include "screen.h"
#include "stdint.h"

void pageFaultException(uint64_t errorCode) {
  uint64_t faultingAddress;
  asm volatile("mov %%cr2, %0" : "=r"(faultingAddress));

  writeStrToScreen("Exception: page fault\n");
  writeStrToScreen("Faulting Address: ");
  writeHex(faultingAddress);
  writeStrToScreen("\nError Code: ");
  writeHex(errorCode);
  writeStrToScreen("\n");

  while (1) {
    asm volatile("hlt");
  }
}

// pageFaultHandler
