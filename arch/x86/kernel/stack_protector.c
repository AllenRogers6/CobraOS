#include <stdint.h>

uintptr_t __stack_chk_guard = 0xDEADBEEF;

void __stack_chk_fail(void) {
  while (1) {
  }
}

void __stack_chk_fail_local(void) { __stack_chk_fail(); }
