#include "gdt.h"
#include "ldt.h"
#include "string.h"
#include "tss.h"

struct gdt_entry gdt[GDT_ENTRIES];

void gdt_set_gate(int index, uint32_t base, uint32_t limit, uint8_t access,
                  uint8_t granularity) {
  gdt[index].base_low = (base & 0xFFFF);
  gdt[index].base_middle = (base >> 16) & 0xFF;
  gdt[index].base_high = (base >> 24) & 0xFF;

  gdt[index].limit_low = (limit & 0xFFFF);
  gdt[index].granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);

  gdt[index].access = access;
}

void init_gdt() {

  gdt_set_gate(0, 0, 0, 0, 0);

  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

  gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

  gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

  gdt_set_gate(5, (uint32_t)&tss, sizeof(tss) - 1, 0x89, 0x00);

  gdt_set_gate(6, (uint32_t)&ldt, sizeof(ldt) - 1, 0x82, 0x00);
}

void load_gdt() {
  struct gdt_ptr gdt_ptr;
  gdt_ptr.limit = sizeof(gdt) - 1;
  gdt_ptr.base = (uint32_t)&gdt;

  asm volatile("lgdt %0" : : "m"(gdt_ptr));
}

void reload_segment_registers() {
  asm volatile("ljmp $0x08, $1f\n\t"
               "1:\n\t");

  asm volatile("mov $0x10, %ax\n\t"
               "mov %ax, %ds\n\t"
               "mov %ax, %es\n\t"
               "mov %ax, %fs\n\t"
               "mov %ax, %gs\n\t"
               "mov %ax, %ss\n\t");
}
