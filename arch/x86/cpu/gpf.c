// #include "get_eip.h"
#include "screen.h"
#include "stdio.h"
#include <stdint.h>

void gpf(void) {
  // uintptr_t eip = get_eip();
  /*uint32_t error_code;
  asm volatile("mov %%ss, %0" : "=r"(error_code));*/

  viprint("Exception: General Protection Fault\n");
  // printf("EIP: %X\n", eip);
  // printf("Error Code: %X\n", error_code);

  while (1) {
    asm volatile("cli; hlt");
  }
}
