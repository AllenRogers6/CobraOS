#include "io.h"
#include "screen.h"
#include <stddef.h>
#include <stdint.h>

#define IOAPIC_REG 0xFEC00000
#define IOAPIC_INDEX (IOAPIC_REG + 0x00)
#define IOAPIC_DATA (IOAPIC_REG + 0x10)

static volatile uint32_t lapic_base;

static inline void lapic_write(uint32_t reg, uint32_t value) {
  *(volatile uint32_t *)(uintptr_t)(lapic_base + reg) = value;
  __asm__ volatile("mfence" ::: "memory");
}

static inline uint32_t lapic_read(uint32_t reg) {
  return *(volatile uint32_t *)(uintptr_t)(lapic_base + reg);
}

void enable_lapic() {
  uint32_t value = lapic_read(0xF0);
  value |= 0x100;
  lapic_write(0xF0, value);
  viprint("LAPIC Enabled.\n");
}

void init_lapic_timer() {
  lapic_write(0x3E0, 0b101);
  lapic_write(0x380, 1000);
  lapic_write(0x320, 0x20 | 0x00020000);
  viprint("LAPIC Timer Initialized.\n");
}

void lapic_eoi() {
  lapic_write(0xB0, 0);
  __asm__ volatile("mfence" ::: "memory");
}

void lapic_handler() {
  viprint("LAPIC Timer Interrupt Triggered\n");
  lapic_eoi();
}

void ioapic_write(uint8_t reg, uint32_t value) {
  volatile uint32_t *ioapic_index = (uint32_t *)(uintptr_t)IOAPIC_INDEX;
  volatile uint32_t *ioapic_data = (uint32_t *)(uintptr_t)IOAPIC_DATA;
  *ioapic_index = reg;
  __asm__ volatile("mfence" ::: "memory");
  *ioapic_data = value;
}

uint32_t ioapic_read(uint8_t reg) {
  volatile uint32_t *ioapic_index = (uint32_t *)(uintptr_t)IOAPIC_INDEX;
  volatile uint32_t *ioapic_data = (uint32_t *)(uintptr_t)IOAPIC_DATA;
  *ioapic_index = reg;
  __asm__ volatile("mfence" ::: "memory");
  return *ioapic_data;
}

void init_ioapic() {
  uint32_t value = ioapic_read(0x00);
  viprint("IOAPIC Version: ");
  hexprint(value);
  ioapic_write(0x10 + (1 * 2), 33);
}

int cpu_supports_apic() {
  uint32_t eax, ebx, ecx, edx;
  __asm__("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
  return (edx & (1 << 9)) != 0;
}

void enable_apic_msr() {
  uint32_t low, high;
  __asm__("rdmsr" : "=a"(low), "=d"(high) : "c"(0x1B));
  high |= 0x800;
  __asm__("wrmsr" : : "c"(0x1B), "a"(low), "d"(high));
  viprint("APIC MSR Enabled.\n");
}

void detect_and_enable_apic() {
  if (!cpu_supports_apic()) {
    viprint("APIC not supported. Halting.\n");
    for (;;) {
    }
  }
  viprint("APIC supported. Enabling...\n");
  enable_apic_msr();

  uint32_t low, high;
  __asm__("rdmsr" : "=a"(low), "=d"(high) : "c"(0x1B));
  lapic_base = (low & 0xFFFFF000);

  if (!lapic_base) {
    viprint("Invalid LAPIC base address. Halting.\n");
    for (;;) {
    }
  }

  viprint("LAPIC Base: ");
  hexprint(lapic_base);
  viprint("\n");
  enable_lapic();
}

void check_apic_vectors() {
  uint32_t svr = lapic_read(0xF0);
  uint32_t timer = lapic_read(0x320);
  uint32_t lint0 = lapic_read(0x350);
  uint32_t lint1 = lapic_read(0x360);
  uint32_t error = lapic_read(0x370);

  viprint("APIC Vector Table:\n");
  viprint("SVR (Spurious): ");
  hexprint(svr);
  viprint("\n");

  viprint("Timer Vector: ");
  hexprint(timer);
  viprint("\n");

  viprint("LINT0 Vector: ");
  hexprint(lint0);
  viprint("\n");

  viprint("LINT1 Vector: ");
  hexprint(lint1);
  viprint("\n");

  viprint("Error Vector: ");
  hexprint(error);
  viprint("\n");
}
