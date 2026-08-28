#ifndef PIT_H
#define PIT_H

#include "registers.h"
#include <stdint.h>

extern volatile uint32_t tick_count;

void pit_init(uint32_t frequency);

void pit_handler(struct registers *r);

void sleep_ticks(uint32_t ticks);

void sleep(uint32_t milliseconds);

void pit_calibrate_with_rtc(uint32_t desired_hz);

uint32_t pit_get_tick_count(void);

#endif /* PIT_H */
