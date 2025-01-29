#include "interrupts.h"
#include "alignmentCheck.h"
#include "apic.h"
#include "boundRangeExceeded.h"
#include "breakpoint.h"
#include "checkingInt.h"
#include "coSegOverrun.h"
#include "controlProtection.h"
#include "debug.h"
#include "devNotFound.h"
#include "divideByZero.h"
#include "doubleFault.h"
#include "floatingPointError.h"
#include "gpf.h"
#include "hypervisorInjection.h"
#include "invalidOpcode.h"
#include "invalidTss.h"
#include "io.h"
#include "keyboard.h"
#include "machineCheck.h"
#include "nmi.h"
#include "overflow.h"
#include "pageFault.h"
#include "pic.h"
#include "screen.h"
#include "security.h"
#include "segNotPresent.h"
#include "simdFloatingPoint.h"
#include "stackSegFault.h"
#include "stdint.h"
#include "tripleFault.h"
#include "virtualization.h"
#include "vmmComms.h"
#include <stdint.h>
#define idtSize 256
#define GATE 0x8E

struct entries {
  uint16_t low;
  uint16_t selector;
  uint8_t ist;
  uint16_t mid;
  uint8_t reserved;
  uint8_t typeAttr;
  uint16_t high;
} __attribute__((packed));

struct idtr {
  uint16_t limit;
  uint64_t base;
} __attribute__((packed)) lidt;

struct entries idt[idtSize];

void set_idt_entry(uint8_t vector, uintptr_t handlerAddress, uint16_t selector,
                   uint8_t type_attr) {
  idt[vector].low = handlerAddress & 0xFFFF;
  idt[vector].selector = selector;
  idt[vector].ist = 0;
  idt[vector].mid = (handlerAddress >> 16) & 0xFFFF;
  idt[vector].reserved = 0;
  idt[vector].typeAttr = type_attr;
  idt[vector].high = (handlerAddress >> 32) & 0xFFFF;
}

void asmIntsOn() { asm volatile("sti"); }
void asmIntsOff() { asm volatile("cli"); }

volatile uint32_t ticks = 0;

void defaultHandler() {
  writeStrToScreen("Unhandled Interrupt\n");
  while (1) {
    asm volatile("hlt");
  }
}

uint32_t idt_base;

void print_idt_entry(uint8_t vector) {
  uint64_t *idt_entry = (uint64_t *)(idt_base + vector * 8);
  uint32_t low_addr = idt_entry[0] & 0xFFFF;
  uint32_t high_addr = (idt_entry[0] >> 16) & 0xFFFF;
  uint8_t flags = (idt_entry[1] >> 8) & 0xFF;
  uint8_t selector = idt_entry[1] & 0xFF;

  writeStrToScreen("IDT Entry for Vector: ");
  writeHex(vector);
  writeStrToScreen("Address low: ");
  writeHex(low_addr);
  writeStrToScreen("Address high: ");
  writeHex(high_addr);
  writeStrToScreen("Flags: ");
  writeHex(flags);
  writeStrToScreen("Selector: ");
  writeHex(selector);
}

struct lidt {
  uint16_t limit;
  uintptr_t base;
};

void load() {

  struct idtr idtr_value = {
      .limit = sizeof(idt) - 1,
      .base = (uintptr_t)&idt,
  };

  asm volatile("lidt (%0)" : : "r"(&idtr_value));
}

void initInts() {
  uint8_t mask = inb(0x21);
  mask &= ~(1 << 1);
  outb(0x21, mask);

  outb(0x60, 0xF4);
  outb(0x64, 0xAE);
  ioapic_write(0x11, 0x21);

  disablePIC();

  writeStrToScreen("Loading IDT\n");
  load();
  writeStrToScreen("Loaded IDT\n");

  asmIntsOn();

  writeStrToScreen("Initializing IDT\n");
  set_idt_entry(0, (uintptr_t)divideByZero, 0x08, GATE);
  set_idt_entry(1, (uintptr_t)debug, 0x08, GATE);
  set_idt_entry(2, (uintptr_t)nmi, 0x08, GATE);
  set_idt_entry(3, (uintptr_t)breakpoint, 0x08, GATE);
  set_idt_entry(4, (uintptr_t)overflow, 0x08, GATE);
  set_idt_entry(5, (uintptr_t)boundRangeExceeded, 0x08, GATE);
  set_idt_entry(6, (uintptr_t)invalidOpcode, 0x08, GATE);
  set_idt_entry(7, (uintptr_t)devNotFound, 0x08, GATE);
  set_idt_entry(8, (uintptr_t)doubleFaultException, 0x08, GATE);
  set_idt_entry(9, (uintptr_t)coSegOverrun, 0x08, GATE);
  set_idt_entry(10, (uintptr_t)invalidTss, 0x08, GATE);
  set_idt_entry(11, (uintptr_t)segNotPresent, 0x08, GATE);
  set_idt_entry(12, (uintptr_t)stackSegFault, 0x08, GATE);
  set_idt_entry(13, (uintptr_t)gpf, 0x08, GATE);
  set_idt_entry(14, (uintptr_t)pageFaultException, 0x08, GATE);
  set_idt_entry(16, (uintptr_t)floatingPointError, 0x08, GATE);
  set_idt_entry(17, (uintptr_t)alignmentCheck, 0x08, GATE);
  set_idt_entry(18, (uintptr_t)machineCheck, 0x08, GATE);
  set_idt_entry(19, (uintptr_t)simdFloatingPoint, 0x08, GATE);
  set_idt_entry(20, (uintptr_t)virtualization, 0x08, GATE);
  set_idt_entry(21, (uintptr_t)controlProtection, 0x08, GATE);
  set_idt_entry(28, (uintptr_t)hypervisorInjection, 0x08, GATE);
  set_idt_entry(29, (uintptr_t)vmmComms, 0x08, GATE);
  set_idt_entry(30, (uintptr_t)security, 0x08, GATE);
  set_idt_entry(32, (uintptr_t)lapic_handler, 0x08, GATE);
  set_idt_entry(33, (uintptr_t)keyboardHandler, 0x08, GATE);

  writeStrToScreen("Interrupts enabled\n");

  print_idt_entry(33);

  areInterruptsEnabled();
  checkInts();
}
