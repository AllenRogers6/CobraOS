#include "io.h"
#include <stdint.h>

uint8_t inb(uint16_t port) {
  uint8_t ret;
  asm volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}

void outb(uint16_t port, uint8_t val) {
  asm volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

void io_wait(void) { asm volatile("outb %%al, $0x80" : : "a"(0)); }

uint32_t inl(uint16_t port) {
  uint32_t result;
  asm volatile("inl %1, %0" : "=a"(result) : "d"(port));
  return result;
}

void outl(uint16_t port, uint32_t value) {
  asm volatile("outl %0, %1" : : "a"(value), "d"(port));
}

void outw(uint16_t port, uint16_t val) {
  asm volatile("outw %0, %1" ::"a"(val), "Nd"(port));
}

uint16_t inw(uint16_t port) {
  uint16_t v;
  asm volatile("inw %1, %0" : "=a"(v) : "Nd"(port));
  return v;
}
