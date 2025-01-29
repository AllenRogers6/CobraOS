#include "io.h"
#include "screen.h"
#include "stdint.h"

#define LAPIC_BASE 0xFEE00000
#define LAPIC_TIMER_REG (LAPIC_BASE + 0x320)
#define LAPIC_LVT_TIMER (LAPIC_BASE + 0x320)
#define LAPIC_LVT_THERMAL (LAPIC_BASE + 0x330)
#define LAPIC_LVT_PERFORMANCE (LAPIC_BASE + 0x340)

#define LAPIC_TIMER_DIVIDE_REG (LAPIC_BASE + 0x3E0)
#define LAPIC_TIMER_INITIAL_COUNT (LAPIC_BASE + 0x380)
#define LAPIC_TIMER_CURRENT_COUNT (LAPIC_BASE + 0x390)

#define LAPIC_EOI_REG (LAPIC_BASE + 0xB0)

#define LAPIC_TICK_INTERVAL 1000

#define LAPIC_ID 0x20
#define LAPIC_VERSION 0x30

#define IOAPIC_REG (0xFEC00000)
#define IOAPIC_INDEX (IOAPIC_REG + 0x00)
#define IOAPIC_DATA (IOAPIC_REG + 0x10)

static inline void lapic_write(uint32_t reg, uint32_t value) {
  *(volatile uint32_t *)(LAPIC_BASE + reg) = value;
}

static inline uint32_t lapic_read(uint32_t reg) {
  return *(volatile uint32_t *)(LAPIC_BASE + reg);
}

void enable_lapic() {
  uint32_t value;

  value = lapic_read(0xF0);
  value |= 0x100;
  lapic_write(0xF0, value);
}

void init_lapic_timer() {
  lapic_write(LAPIC_TIMER_DIVIDE_REG, 0x00000003);

  lapic_write(LAPIC_TIMER_INITIAL_COUNT, LAPIC_TICK_INTERVAL);

  lapic_write(LAPIC_LVT_TIMER, 0x00010000);

  lapic_write(LAPIC_TIMER_REG, 0x00000001);
}

void lapic_eoi() { lapic_write(LAPIC_EOI_REG, 0); }

void lapic_handler() {
  writeStrToScreen("LAPIC Timer Interrupt Triggered\n");
  lapic_eoi();
}

void ioapic_write(uint8_t reg, uint32_t value) {
  outl(IOAPIC_INDEX, reg);
  outl(IOAPIC_DATA, value);
}

uint32_t ioapic_read(uint8_t reg) {
  outl(IOAPIC_INDEX, reg);
  return inl(IOAPIC_DATA);
}

void init_ioapic() {
  uint32_t value = ioapic_read(0x00);
  writeStrToScreen("IOAPIC Version: ");
  writeHex(value);

  ioapic_write(0x10 + (1 * 2), 33);
}
