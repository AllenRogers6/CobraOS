#include "registers.h"
#include "screen.h"
#include "stdio.h"
#include <stdint.h>

void debug_stack(uint32_t *esp) {
  printf("Stack Dump at ESP: %X\n", esp);
  for (int i = 0; i < 5; i++) {
    printf("[%d] %X\n", i, esp[i]);
  }
}

void invalid_opcode(registers_t *regs) {
  viprint("Exception: Invalid opcode\n");
  /*printf("Faulting EIP: %X\n", regs->eip);

  debug_stack((uint32_t *)regs->esp);*/

  viprint("Reseting\n");
  asm volatile("jmp 0xFFFF0000");
  /*while (1) {
    asm volatile("cli; hlt");
  }*/
}
