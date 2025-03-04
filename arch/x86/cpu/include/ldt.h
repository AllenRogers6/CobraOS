#ifndef LDT_H
#define LDT_H

#include <stdint.h>

#define LDT_ENTRIES 2

struct ldt_entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t granularity;
  uint8_t base_high;
} __attribute__((packed));

extern struct ldt_entry ldt[LDT_ENTRIES];

void gdt_set_gate_ldt(struct ldt_entry *entry, uint32_t base, uint32_t limit,
                      uint8_t access, uint8_t granularity);
void init_ldt();
void load_ldt();

#endif
