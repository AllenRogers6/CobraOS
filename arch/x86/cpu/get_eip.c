#include "screen.h"
#include <stdint.h>

uintptr_t get_eip() {
  uintptr_t eip;
  __asm__ volatile("call 1f; 1: pop %0" : "=r"(eip));
  return eip;
}
