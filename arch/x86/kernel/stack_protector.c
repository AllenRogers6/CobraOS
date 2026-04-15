#include <stdint.h>

// The stack canary value (must be random at runtime)
uintptr_t __stack_chk_guard = 0xDEADBEEF;

// Called when stack check fails
void __stack_chk_fail(void) {
  // Halt or panic
  while (1) {
  }
}

// For some GCC versions, __stack_chk_fail_local is an alias
void __stack_chk_fail_local(void) { __stack_chk_fail(); }
