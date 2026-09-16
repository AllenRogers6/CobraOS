#include "timer.h"
#include "pit.h"
#include "screen.h"
#include "stdio.h"
#include <stdint.h>

static struct timer *timer_queue = NULL;
static bool timer_ready = false;

static volatile int timer_lock = 0;

static inline void lock_acquire(uint32_t *flags) {
  asm volatile("pushfl; popl %0; cli" : "=r"(*flags)::"memory");
  while (__sync_lock_test_and_set(&timer_lock, 1)) {
    while (timer_lock)
      asm volatile("pause");
  }
}

static inline void lock_release(uint32_t flags) {
  __sync_lock_release(&timer_lock);
  if (flags & (1u << 9))
    asm volatile("sti" ::: "memory");
}

static void queue_insert(struct timer *t) {
  struct timer **pp = &timer_queue;
  while (*pp && (*pp)->expires <= t->expires)
    pp = &(*pp)->next;
  t->next = *pp;
  *pp = t;
}

static void queue_remove(struct timer *t) {
  struct timer **pp = &timer_queue;
  while (*pp) {
    if (*pp == t) {
      *pp = t->next;
      t->next = NULL;
      return;
    }
    pp = &(*pp)->next;
  }
}

void timer_tick(void) {
  const uint32_t now = pit_get_tick_count();
  uint32_t flags;

  lock_acquire(&flags);

  while (timer_queue && (int32_t)(timer_queue->expires - now) <= 0) {
    struct timer *t = timer_queue;
    timer_queue = t->next;
    t->next = NULL;

    if (t->period) {
      t->expires = now + t->period;
      queue_insert(t);
    } else {
      t->active = false;
    }

    lock_release(flags);

    if (t->fn)
      t->fn(t->arg);

    lock_acquire(&flags);
  }

  lock_release(flags);
}

void timer_init(void) {
  if (timer_ready)
    return;

  uint32_t flags;
  lock_acquire(&flags);
  timer_queue = NULL;
  timer_ready = true;
  lock_release(flags);

  viprint("timer: ready (tick source = pit)\n");
}

uint32_t timer_ticks(void) { return pit_get_tick_count(); }

uint32_t timer_ms(void) { return TICKS_TO_MS(pit_get_tick_count()); }

void timer_setup(struct timer *t, timer_fn_t fn, void *arg, uint32_t period) {
  t->expires = 0;
  t->period = period;
  t->fn = fn;
  t->arg = arg;
  t->active = false;
  t->next = NULL;
}

void timer_add(struct timer *t, uint32_t delay_ticks) {
  uint32_t flags;
  lock_acquire(&flags);

  if (t->active)
    queue_remove(t);

  t->expires = pit_get_tick_count() + (delay_ticks ? delay_ticks : 1);
  t->active = true;
  queue_insert(t);

  lock_release(flags);
}

void timer_add_ms(struct timer *t, uint32_t delay_ms) {
  timer_add(t, MS_TO_TICKS(delay_ms));
}

int timer_cancel(struct timer *t) {
  uint32_t flags;
  int was_active;

  lock_acquire(&flags);
  was_active = t->active;
  if (was_active) {
    queue_remove(t);
    t->active = false;
  }
  lock_release(flags);

  return was_active ? 0 : -1;
}
