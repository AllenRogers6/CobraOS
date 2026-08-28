#include "interrupts.h"
#include "io.h"
#include "pic.h"
#include "registers.h"
#include "screen.h"
#include "stdio.h"
#include "string.h"
#include <stdint.h>

#define IDT_SIZE 256
#define INT_GATE 0x8E
#define TRAP_GATE 0x8F
#define DPL0 0x00
#define DPL3 0x60

#define PIC1_CMD 0x20
#define PIC2_CMD 0xA0
#define PIC_EOI 0x20
#define PIC_READ_ISR 0x0B

#define IRQ_BASE 32
#define IRQ_COUNT 16
#define SYSCALL_VECTOR 0x80
#define MAX_SYSCALLS 64

struct idt_entry {
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

static struct idt_entry idt[IDT_SIZE];
static struct idtr idtp __attribute__((aligned(16)));

extern uintptr_t isr_stub_table[256];

static irq_handler_t irq_handlers[IRQ_COUNT];
static syscall_handler_t syscall_table[MAX_SYSCALLS];

static void set_idt_entry(int vector, uintptr_t handler, uint16_t selector,
                          uint8_t type_attr) {
  idt[vector].low = handler & 0xFFFF;
  idt[vector].selector = selector;
  idt[vector].reserved = 0;
  idt[vector].type_attr = type_attr;
  idt[vector].high = (handler >> 16) & 0xFFFF;
}

void asm_ints_on(void) {
  asm volatile("sti");
  viprint("Interrupts enabled\n");
}

void asm_ints_off(void) {
  asm volatile("cli");
  viprint("Interrupts disabled\n");
}

static void load_idt(void) {
  idtp.limit = sizeof(idt) - 1;
  idtp.base = (uintptr_t)&idt;
  asm volatile("lidt %0" : : "m"(idtp) : "memory");
  viprint("Loaded IDT\n");
}

static uint16_t pic_read_isr(void) {
  outb(PIC1_CMD, PIC_READ_ISR);
  uint8_t pic1 = inb(PIC1_CMD);

  outb(PIC2_CMD, PIC_READ_ISR);
  uint8_t pic2 = inb(PIC2_CMD);

  return (pic2 << 8) | pic1;
}

static void pic_send_eoi(uint8_t irq) {
  if (irq >= 8)
    outb(PIC2_CMD, PIC_EOI);
  outb(PIC1_CMD, PIC_EOI);
}

static const char *exception_names[] = {"Divide by zero",
                                        "Debug",
                                        "NMI",
                                        "Breakpoint",
                                        "Overflow",
                                        "Bound range exceeded",
                                        "Invalid opcode",
                                        "Device not available",
                                        "Double fault",
                                        "Coprocessor segment overrun",
                                        "Invalid TSS",
                                        "Segment not present",
                                        "Stack-segment fault",
                                        "General protection fault",
                                        "Page fault",
                                        "Reserved",
                                        "x87 FPU error",
                                        "Alignment check",
                                        "Machine check",
                                        "SIMD FPU exception",
                                        "Virtualization exception",
                                        "Control protection exception",
                                        "Reserved",
                                        "Reserved",
                                        "Reserved",
                                        "Reserved",
                                        "Reserved",
                                        "Reserved",
                                        "Hypervisor injection exception",
                                        "VMM communication exception",
                                        "Security exception"};

static void print_regs(struct registers *r) {
  viprint("EAX=");
  hexprint(r->eax);
  viprint(" EBX=");
  hexprint(r->ebx);
  viprint(" ECX=");
  hexprint(r->ecx);
  viprint(" EDX=");
  hexprint(r->edx);

  viprint("\nESI=");
  hexprint(r->esi);
  viprint(" EDI=");
  hexprint(r->edi);
  viprint(" EBP=");
  hexprint(r->ebp);
  viprint(" ESP=");
  hexprint(r->esp);

  viprint("\nEIP=");
  hexprint(r->eip);
  viprint(" CS=");
  hexprint(r->cs);
  viprint(" EFLAGS=");
  hexprint(r->eflags);
  viprint(" USERESP=");
  hexprint(r->useresp);
  viprint(" SS=");
  hexprint(r->ss);
  viprint("\n");
}

void exception_handler(struct registers *r) {
  uint32_t vector = r->int_no;

  if (vector < 31)
    viprint(exception_names[vector]);
  else
    viprint("Unknown exception");

  viprint(" Exception ");
  hexprint(vector);
  viprint(" err=");
  hexprint(r->err_code);
  viprint("\n");

  print_regs(r);

  if (vector == 14) { /* #PF */
    uint32_t cr2;
    asm volatile("mov %%cr2, %0" : "=r"(cr2));
    viprint("CR2=");
    hexprint(cr2);
    viprint("\n");
  }

  viprint("Halting.\n");
  asm volatile("cli; hlt");
}

void irq_dispatch(struct registers *r) {
  uint8_t irq = r->int_no - IRQ_BASE;

  if (irq >= IRQ_COUNT) {
    viprint("Invalid IRQ vector\n");
    return;
  }

  if (irq == 7 || irq == 15) {
    uint16_t isr = pic_read_isr();
    if (!(isr & (1 << irq))) {
      /*
       * Spurious slave IRQ15 requires EOI to master,
       * but not to slave.
       */
      if (irq == 15)
        outb(PIC1_CMD, PIC_EOI);
      return;
    }
  }

  if (irq_handlers[irq]) {
    irq_handlers[irq](r);
  } else {
    viprint("Unhandled IRQ ");
    hexprint(irq);
    viprint("\n");
  }

  pic_send_eoi(irq);
}

static uint32_t syscall_dispatch(struct registers *r) {
  uint32_t num = r->eax;

  if (num >= MAX_SYSCALLS || !syscall_table[num])
    return 0xFFFFFFFF; /* -ENOSYS */

  return syscall_table[num](r->ebx, r->ecx, r->edx, r->esi);
}

void interrupt_dispatch(struct registers *r) {

  uint32_t vector = r->int_no;

  if (vector < 32) {
    exception_handler(r);
  } else if (vector >= IRQ_BASE && vector < IRQ_BASE + IRQ_COUNT) {
    irq_dispatch(r);
  } else if (vector == SYSCALL_VECTOR) {
    r->eax = syscall_dispatch(r);
  } else {
    viprint("Unhandled interrupt: ");
    hexprint(vector);
    viprint("\n");
    asm volatile("cli; hlt");
  }
}

int irq_register_handler(uint8_t irq, irq_handler_t handler) {
  if (irq >= IRQ_COUNT || !handler)
    return -1;
  if (irq_handlers[irq])
    return -1; /* already occupied */

  irq_handlers[irq] = handler;
  return 0;
}

void irq_unregister_handler(uint8_t irq) {
  if (irq < IRQ_COUNT)
    irq_handlers[irq] = (irq_handler_t)0;
}

int syscall_register(uint8_t num, syscall_handler_t handler) {
  if (num >= MAX_SYSCALLS || !handler)
    return -1;
  if (syscall_table[num])
    return -1;

  syscall_table[num] = handler;
  return 0;
}

void init_ints(void) {
  viprint("Setting IDT entries\n");

  for (int i = 0; i < IDT_SIZE; i++) {
    uint8_t attr = INT_GATE;

    if (i == 3)
      attr = TRAP_GATE | DPL3;

    if (i == SYSCALL_VECTOR)
      attr = INT_GATE | DPL3;

    set_idt_entry(i, isr_stub_table[i], 0x08, attr);
  }

  viprint("Remapping PIC\n");
  remap();

  viprint("Loading IDT\n");
  load_idt();
}

void verify_idt(void) {
  struct idtr tmp;
  asm volatile("sidt %0" : "=m"(tmp));

  if (tmp.base == idtp.base && tmp.limit == idtp.limit)
    viprint("IDT OK\n");
  else
    viprint("IDT mismatch!\n");
}

void read_esp(void) {
  uint32_t esp;
  asm volatile("mov %%esp, %0" : "=r"(esp));
  viprint("ESP: ");
  hexprint(esp);
}
