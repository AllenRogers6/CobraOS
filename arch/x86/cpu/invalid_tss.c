#include "screen.h"
#include "stdio.h"

void invalid_tss(void) {
  uint32_t error_code;

  viprint("Exception: Invalid tss\n");
  asm volatile("movl %%ss, %0" : "=r"(error_code)); // Read the error code

  printf("Error Code: 0x%x\n", error_code);

  while (1) {
    asm volatile("hlt");
  }
}
