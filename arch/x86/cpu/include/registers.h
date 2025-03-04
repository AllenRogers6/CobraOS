#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

typedef struct registers {
  uint32_t ds;                                     // Data segment selector
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha
  uint32_t int_no, err_code; // Interrupt number and error code (if applicable)
  uint32_t eip, cs, eflags, useresp, ss; // Pushed automatically by the CPU
} registers_t;

#endif // !DEBUG
