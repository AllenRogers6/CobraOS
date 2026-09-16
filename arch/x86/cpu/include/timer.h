#ifndef _TIMER_H
#define _TIMER_H

#include <stdbool.h>
#include <stdint.h>

#define PIT_HZ 100
#define MS_TO_TICKS(ms) (((uint32_t)(ms) * PIT_HZ + 999) / 1000)
#define TICKS_TO_MS(t) ((uint32_t)(t) * 1000 / PIT_HZ)

typedef void (*timer_fn_t)(void *arg);

struct timer {
  uint32_t expires;
  uint32_t period;
  timer_fn_t fn;
  void *arg;
  bool active;
  struct timer *next;
};

void timer_init(void);

void timer_tick(void);

uint32_t timer_ticks(void);
uint32_t timer_ms(void);

void timer_setup(struct timer *t, timer_fn_t fn, void *arg, uint32_t period);
void timer_add(struct timer *t, uint32_t delay_ticks);
void timer_add_ms(struct timer *t, uint32_t delay_ms);
int timer_cancel(struct timer *t);

#endif
