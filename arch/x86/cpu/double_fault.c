// #include "get_eip.h"
#include "registers.h"
#include "screen.h"
#include "stdio.h"

void double_fault_exception() {
  asm volatile("pusha");

  // uint32_t eip = get_eip();
  uint32_t error_code;
  asm volatile("mov %%ss, %0" : "=r"(error_code));

  viprint("Exception: Double Fault\n");
  // printf("EIP: %X\n", eip);
  printf("Error Code: %X\n", error_code);

  uint16_t cs, ds, es, fs, gs, ss;
  asm volatile("mov %%cs, %0" : "=r"(cs));
  asm volatile("mov %%ds, %0" : "=r"(ds));
  asm volatile("mov %%es, %0" : "=r"(es));
  asm volatile("mov %%fs, %0" : "=r"(fs));
  asm volatile("mov %%gs, %0" : "=r"(gs));
  asm volatile("mov %%ss, %0" : "=r"(ss));
  printf("CS: %X, DS: %X, ES: %X, FS: %X, GS: %X, SS: %X\n", cs, ds, es, fs, gs,
         ss);

  asm volatile("popa");
  asm volatile("iret");

  while (1) {
    asm volatile("cli; hlt");
  }
}
