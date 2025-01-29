#include "screen.h"
#include "stdint.h"

int areInterruptsEnabled() {
  uint64_t eflags;
  asm volatile("pushf\n\t"
               "pop %0"
               : "=r"(eflags));

  return eflags & 0x200;
}

void checkInts() {
  if (areInterruptsEnabled()) {
    writeStrToScreen("IntOn\n");
  } else {
    writeStrToScreen("intOff\n");
  }
}
