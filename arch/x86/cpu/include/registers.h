#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

struct registers {
  uint32_t gs, fs, es, ds;
  uint32_t edi, esi, ebp, esp;
  uint32_t ebx, edx, ecx, eax;
  uint32_t int_no, err_code;
  uint32_t eip, cs, eflags;
  uint32_t useresp, ss;
} __attribute__((packed));

#endif // !DEBUG
