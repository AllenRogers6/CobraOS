#ifndef _SHUTDOWN_H
#define _SHUTDOWN_H

#include <stdbool.h>
#include <stdint.h>

enum shutdown_mode {
  SHUTDOWN_POWEROFF = 0,
  SHUTDOWN_REBOOT = 1,
  SHUTDOWN_HALT = 2
};

void schedule_shutdown(enum shutdown_mode mode, uint32_t delay_ms,
                       const char *msg);

void cancel_shutdown(void);
bool shutdown_is_pending(void);

uint32_t shutdown_remaining_ms(void);
void schedule_default_shutdown(void);

#endif
