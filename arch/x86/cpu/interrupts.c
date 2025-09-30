#include "interrupts.h"
#include "alignment_check.h"
#include "apic.h"
#include "bound_range_exceeded.h"
#include "breakpoint.h"
#include "checking_int.h"
#include "co_seg_overrun.h"
#include "control_protection.h"
#include "debug.h"
#include "dev_not_found.h"
#include "divide_by_zero.h"
#include "double_fault.h"
#include "floating_point_error.h"
#include "gpf.h"
#include "hypervisor_injection.h"
#include "invalid_opcode.h"
#include "invalid_tss.h"
#include "io.h"
#include "keyboard.h"
#include "machine_check.h"
#include "nmi.h"
#include "overflow.h"
#include "page_fault.h"
#include "pic.h"
#include "pit.h"
#include "registers.h"
#include "screen.h"
#include "security.h"
#include "seg_not_present.h"
#include "simd_floating_point.h"
#include "stack_seg_fault.h"
#include "stdio.h"
#include "string.h"
#include "triple_fault.h"
#include "virtualization.h"
#include "vmm_comms.h"
#include <stdint.h>

#define IDT_SIZE 256
#define INT_GATE 0x8E
#define TRAP_GATE 0x8F
#define TASK_GATE 0x5
#define DPL0 0x00
#define DPL3 0x60

struct entries {
  uint16_t low;
  uint16_t selector;
  uint8_t reserved;
  uint8_t type_attr;
  uint16_t high;
} __attribute__((packed));

struct idtr {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed));

struct entries idt[IDT_SIZE];
struct idtr idtp __attribute__((aligned(16)));

void set_idt_entry(int vector, uintptr_t handler_address, uint16_t selector,
                   uint8_t type_attr) {
  idt[vector].low = handler_address & 0xFFFF;
  idt[vector].selector = selector;
  idt[vector].reserved = 0;
  idt[vector].type_attr = type_attr;
  idt[vector].high = (handler_address >> 16) & 0xFFFF;
}

void asm_ints_on() {
  asm volatile("sti");
  viprint("Interrupts enabled\n");
}

void asm_ints_off() {
  asm volatile("cli");
  viprint("Interrupts disabled\n");
}

void exception_handler() {
  uint32_t esp;
  asm volatile("mov %%esp, %0" : "=r"(esp));
  viprint("Probable uncaught exception\n");
  viprint("ESP:");
  hexprint(esp);

  outb(0x20, 0x20);
}

void default_handler(int vector) {
  viprint("Unhandled Interrupt: ");
  hexprint(vector);
  viprint("\n");

  exception_handler();

  if (vector == 0x27 || vector == 0x2F) {
    return;
  }

  while (1)
    asm volatile("cli; hlt");
}

void print_idt_entry(uint8_t vector) {
  uintptr_t *idt_base = NULL;
  uintptr_t *idt_entry = (uintptr_t *)(idt_base + vector * 8);
  uintptr_t low_addr = idt_entry[0] & 0xFFFF;
  uintptr_t high_addr = (idt_entry[0] >> 16) & 0xFFFF;
  uintptr_t flags = (idt_entry[1] >> 8) & 0xFF;
  uintptr_t selector = idt_entry[1] & 0xFF;

  viprint("IDT Entry for Vector: ");
  hexprint(vector);
  viprint("Address low: ");
  hexprint(low_addr);
  viprint("Address high: ");
  hexprint(high_addr);
  viprint("Flags: ");
  hexprint(flags);
  viprint("Selector: ");
  hexprint(selector);
}

void load() {
  idtp.limit = sizeof(idt) - 1;
  idtp.base = (uintptr_t)&idt;
  asm volatile("lidt %0" : : "m"(idtp) : "memory");
  has_loaded();
  viprint("Loaded IDT\n");
}

void verify_idt() {
  struct idtr tmp;
  __asm__ volatile("sidt %0" : "=m"(tmp));

  viprint("Stored IDT Base: ");
  hexprint(tmp.base);
  viprint("Expected IDT Base: ");
  hexprint(idtp.base);

  if (tmp.base == idtp.base && tmp.limit == idtp.limit) {
    viprint("IDT loaded correctly\n");
  } else {
    viprint("IDT loading failed\n");
    while (1)
      asm volatile("hlt");
  }
}

void read_handler_for_vec0() {
  viprint("Handler for int 0: ");
  hexprint((uintptr_t)divide_by_zero);
  viprint("\n");
}

void read_handler_for_vec33() {
  viprint("Handler for int 0: ");
  hexprint((uintptr_t)keyboard_handler);
  viprint("\n");
}

void read_esp() {
  uint32_t esp;
  asm volatile("mov %%esp, %0" : "=r"(esp));
  viprint("Stack Pointer: ");
  hexprint(esp);
  viprint("\n");
}

void cmp_base_lim() {
  struct idtr tmp;
  asm volatile("sidt %0" : "=m"(tmp));

  viprint("IDT Base: ");
  hexprint(idtp.base);
  viprint("\nIDT Limit: ");
  hexprint(idtp.limit);
  viprint("\n");

  viprint("Stored IDT Base: ");
  hexprint(tmp.base);
  viprint("Stored IDT Limit: ");
  hexprint(tmp.limit);
}

void init_ints() {
  viprint("Setting IDT entries\n");

  for (int i = 0; i < IDT_SIZE; i++) {
    set_idt_entry(i, (uintptr_t)default_handler, 0x08, INT_GATE);
  }

  set_idt_entry(0, (uintptr_t)divide_by_zero, 0x08, TRAP_GATE);
  set_idt_entry(1, (uintptr_t)debug, 0x08, TRAP_GATE);
  set_idt_entry(2, (uintptr_t)nmi, 0x08, INT_GATE);
  set_idt_entry(3, (uintptr_t)breakpoint, 0x08, TRAP_GATE | DPL3);
  set_idt_entry(4, (uintptr_t)overflow, 0x08, TRAP_GATE | DPL3);
  set_idt_entry(5, (uintptr_t)bound_range_exceeded, 0x08, TRAP_GATE);
  set_idt_entry(6, (uintptr_t)invalid_opcode, 0x08, TRAP_GATE);
  set_idt_entry(7, (uintptr_t)dev_not_found, 0x08, TRAP_GATE);
  set_idt_entry(8, (uintptr_t)double_fault_exception, 0x08, INT_GATE);
  set_idt_entry(9, (uintptr_t)co_seg_overrun, 0x08, INT_GATE);
  set_idt_entry(10, (uintptr_t)invalid_tss, 0x08, INT_GATE);
  set_idt_entry(11, (uintptr_t)seg_not_present, 0x08, INT_GATE);
  set_idt_entry(12, (uintptr_t)stack_seg_fault, 0x08, INT_GATE);
  set_idt_entry(13, (uintptr_t)gpf, 0x08, INT_GATE);
  set_idt_entry(14, (uintptr_t)page_fault_exception, 0x08, INT_GATE);
  set_idt_entry(16, (uintptr_t)floating_point_error, 0x08, INT_GATE);
  set_idt_entry(17, (uintptr_t)alignment_check, 0x08, INT_GATE);
  set_idt_entry(19, (uintptr_t)simd_floating_point, 0x08, INT_GATE);
  set_idt_entry(18, (uintptr_t)machine_check, 0x08, INT_GATE);
  set_idt_entry(20, (uintptr_t)virtualization, 0x08, INT_GATE);
  set_idt_entry(21, (uintptr_t)control_protection, 0x08, INT_GATE);
  set_idt_entry(28, (uintptr_t)hypervisor_injection, 0x08, INT_GATE);
  set_idt_entry(29, (uintptr_t)vmm_comms, 0x08, INT_GATE);
  set_idt_entry(30, (uintptr_t)security, 0x08, INT_GATE);
  set_idt_entry(32, (uintptr_t)pit_handler, 0x08, INT_GATE);
  set_idt_entry(33, (uintptr_t)keyboard_handler, 0x08, INT_GATE | DPL0);

  viprint("Remapping\n");
  remap();

  viprint("Loading IDT\n");

  load();

  asm_ints_on();
}
