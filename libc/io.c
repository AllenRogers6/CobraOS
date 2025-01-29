#include "io.h"
#include <stdint.h>

uint8_t inb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
  return ret;
}

void outb(uint16_t port, uint8_t val) {
  __asm__ volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

void io_wait(void) { outb(0x80, 0); }

uint32_t inl(uint16_t port) {
  uint32_t result;
  asm volatile("inl %1, %0" : "=a"(result) : "d"(port));
  return result;
}

void outl(uint16_t port, uint32_t value) {
  asm volatile("outl %0, %1" : : "a"(value), "d"(port));
}
