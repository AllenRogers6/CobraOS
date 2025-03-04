#include "ldt.h"
#include "gdt.h"
#include "string.h"

struct ldt_entry ldt[LDT_ENTRIES];

void init_ldt() {
  memset(ldt, 0, sizeof(ldt));

  gdt_set_gate_ldt(&ldt[0], 0, 0xFFFFFFFF, 0x9A, 0xCF);

  gdt_set_gate_ldt(&ldt[1], 0, 0xFFFFFFFF, 0x92, 0xCF);
}

void gdt_set_gate_ldt(struct ldt_entry *entry, uint32_t base, uint32_t limit,
                      uint8_t access, uint8_t granularity) {
  entry->base_low = (base & 0xFFFF);
  entry->base_middle = (base >> 16) & 0xFF;
  entry->base_high = (base >> 24) & 0xFF;

  entry->limit_low = (limit & 0xFFFF);
  entry->granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);

  entry->access = access;
}

void load_ldt() { asm volatile("lldt %0" : : "r"((uint16_t)LDT_SEGMENT)); }
