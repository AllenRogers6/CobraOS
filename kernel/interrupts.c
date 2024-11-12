#include "include/interrupts.h"
#include "include/checkingInt.h"
#include "include/pic.h"

#define idtSize 256

struct entries {
  uint16_t offset_low;
  uint16_t selector;
  uint8_t zero;
  uint8_t type_attr;
  uint16_t offset_high;
} __attribute__((packed));

struct idtr {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed)) lidt;

struct entries idt[idtSize];

void set_idt_entry(int n, uint32_t isr_address, uint16_t selector,
                   uint8_t type_attr) {
  idt[n].offset_low = isr_address & 0xFFFF;
  idt[n].selector = selector;
  idt[n].zero = 0;
  idt[n].type_attr = type_attr;
  idt[n].offset_high = (isr_address >> 16) & 0xFFFF;
}

volatile uint32_t ticks = 0;

void timer_isr() { ticks++; }

uint32_t get_ticks() { return ticks; }

void init_interrupts() {

  set_idt_entry(32, (uint32_t)timer_isr, 0x08, 0x8E);

  remap_pic();

  asm volatile("lidt (%0)" : : "r"(&lidt));

  are_interrupts_enabled();
  check_interrupts();
}

void enable_interrupts() { asm volatile("sti"); }

void disable_interrupts() { asm volatile("cli"); }
