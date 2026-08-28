#include "pit.h"
#include "io.h"
#include "registers.h"
#include "screen.h"
#include "task.h"
#include <stdint.h>

volatile uint32_t tick_count = 0;
static uint32_t pit_hz = 100;
static uint32_t current_divisor = 0;

static inline void cmos_write(uint8_t reg, uint8_t val) {
  outb(0x70, (reg & 0x7F) | 0x80); // disable NMI during write
  outb(0x71, val);
}

static inline uint8_t cmos_read(uint8_t reg) {
  outb(0x70, reg & 0x7F); // keep NMI enabled
  return inb(0x71);
}

#define RTC_RATE_1024HZ 0x06 // 1024 Hz periodic interrupt rate

static void rtc_enable_periodic(uint8_t rate) {
  uint8_t regA = cmos_read(0x0A);
  regA = (regA & 0xF0) | (rate & 0x0F);
  cmos_write(0x0A, regA);

  uint8_t regB = cmos_read(0x0B);
  regB |= 0x40; // set Periodic Interrupt Enable (PIE)
  cmos_write(0x0B, regB);
}

static void rtc_disable_periodic(void) {
  uint8_t regB = cmos_read(0x0B);
  regB &= ~0x40;
  cmos_write(0x0B, regB);
}

static void rtc_clear_pf(void) {
  cmos_read(0x0C); // reading register C clears PF
}

static void rtc_wait_for_pf(void) {
  while (!(cmos_read(0x0C) & 0x40)) {
    /* spin */
  }
}

void pit_init(uint32_t frequency) {
  pit_hz = frequency;
  current_divisor = 1193182 / frequency;
  outb(0x43, 0x36); // Channel 0, lobyte/hibyte, rate generator
  outb(0x40, current_divisor & 0xFF);        // Low byte
  outb(0x40, (current_divisor >> 8) & 0xFF); // High byte
}

void pit_handler(struct registers *r) {
  (void)r;
  tick_count++;
  /* EOI is sent by the dispatcher after this returns. */
}

void pit_calibrate_with_rtc(uint32_t desired_hz) {
  if (desired_hz == 0 || current_divisor == 0)
    return;

  rtc_enable_periodic(RTC_RATE_1024HZ);

  rtc_clear_pf();

  uint32_t start_ticks = tick_count;

  for (int i = 0; i < 1024; i++) {
    rtc_wait_for_pf();
    rtc_clear_pf(); // clear immediately
  }

  uint32_t end_ticks = tick_count;

  rtc_disable_periodic();

  uint32_t measured_ticks = end_ticks - start_ticks;
  if (measured_ticks == 0)
    return;

  uint32_t actual_base_freq = measured_ticks * current_divisor;

  uint32_t new_divisor = actual_base_freq / desired_hz;
  if (new_divisor < 2)
    new_divisor = 2; // avoid division by zero

  current_divisor = new_divisor;
  pit_hz = desired_hz;

  outb(0x43, 0x36);
  outb(0x40, new_divisor & 0xFF);
  outb(0x40, (new_divisor >> 8) & 0xFF);
}

void sleep_ticks(uint32_t ticks) {
  if (ticks == 0)
    return;

  uint32_t target = tick_count + ticks;
  while ((int32_t)(tick_count - target) < 0) {
    asm volatile("sti; hlt; cli");
  }
  asm volatile("sti");
}

void sleep(uint32_t milliseconds) {
  if (milliseconds == 0)
    return;

  uint32_t ticks = (milliseconds * pit_hz + 999) / 1000;
  sleep_ticks(ticks);
}

uint32_t pit_get_tick_count(void) { return tick_count; }
