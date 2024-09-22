#include "include/interrupts.h"
#include "stdint.h"

void lidt(void *idt_addr) {

  struct idtr idtReg;

  idtReg.limit = 0xFFFF;
  idtReg.base = (uint64_t)&idt;

  asm("lidt %0" : : "r"(&idtReg));
}
