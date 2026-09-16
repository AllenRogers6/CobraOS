#include "shutdown.h"
#include "io.h"
#include "pit.h"
#include "screen.h"
#include "stdio.h"
#include "string.h"
#include "timer.h"
#include <stdbool.h>
#include <stdint.h>

#define SHUTDOWN_DEFAULT_MS (10u * 60u * 1000u)

static bool sd_active = false;
static enum shutdown_mode sd_mode = SHUTDOWN_POWEROFF;
static uint32_t sd_deadline = 0;
static struct timer sd_timer;
static char sd_msg[96];

static void hw_poweroff(void) {
  outw(0x604, 0x2000);
  outw(0xB004, 0x2000);
  outw(0x8900, 0x2000);

  for (;;)
    asm volatile("cli; hlt");
}

static void hw_reboot(void) {
  while (inb(0x64) & 0x02) {
  }
  outb(0x64, 0xFE);

  for (;;)
    asm volatile("cli; hlt");
}

static void shutdown_fire(void *arg) {
  (void)arg;
  if (!sd_active)
    return;

  sd_active = false;

  viprint("\nSystem going down: ");
  viprint(sd_msg);
  viprint("\n");

  switch (sd_mode) {
  case SHUTDOWN_POWEROFF:
    hw_poweroff();
    break;
  case SHUTDOWN_REBOOT:
    hw_reboot();
    break;
  case SHUTDOWN_HALT:
  default:
    for (;;)
      asm volatile("cli; hlt");
  }
}

void schedule_shutdown(enum shutdown_mode mode, uint32_t delay_ms,
                       const char *msg) {
  cancel_shutdown();

  sd_active = true;
  sd_mode = mode;

  if (msg && msg[0]) {
    strncpy(sd_msg, msg, sizeof(sd_msg) - 1);
    sd_msg[sizeof(sd_msg) - 1] = '\0';
  } else {
    strcpy(sd_msg, "scheduled shutdown");
  }

  timer_setup(&sd_timer, shutdown_fire, NULL, 0);
  timer_add_ms(&sd_timer, delay_ms);

  uint32_t hz = pit_get_hz();
  uint32_t delay_ticks = (delay_ms * hz + 999) / 1000;
  if (delay_ticks == 0)
    delay_ticks = 1;
  sd_deadline = pit_get_tick_count() + delay_ticks;
}

void cancel_shutdown(void) {
  if (sd_active) {
    timer_cancel(&sd_timer);
    sd_active = false;
  }
}

bool shutdown_is_pending(void) { return sd_active; }

uint32_t shutdown_remaining_ms(void) {
  if (!sd_active)
    return 0;

  uint32_t now = pit_get_tick_count();
  int32_t left_ticks = (int32_t)(sd_deadline - now);
  if (left_ticks <= 0)
    return 0;

  uint32_t hz = pit_get_hz();
  return ((uint32_t)left_ticks * 1000) / hz;
}

void schedule_default_shutdown(void) {
  if (sd_active)
    return;
  schedule_shutdown(SHUTDOWN_POWEROFF, SHUTDOWN_DEFAULT_MS,
                    "automatic 10-minute shutdown");
}
