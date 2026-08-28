#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>

/*
 * spinlock.h — Simple x86 spinlock with interrupt-save variants.
 *
 * Two usage patterns:
 *
 * 1. Between two kernel threads (interrupts already off, or you know
 *    the ISR won't touch the same data):
 *
 *      spinlock_t lock = SPINLOCK_INIT;
 *      spinlock_acquire(&lock);
 *      ... critical section ...
 *      spinlock_release(&lock);
 *
 * 2. Between kernel code and an ISR (the common case — heap, screen, etc.):
 *
 *      spinlock_t lock = SPINLOCK_INIT;
 *      uint32_t flags;
 *      spinlock_acquire_irq(&lock, &flags);   // disables IRQs atomically
 *      ... critical section ...
 *      spinlock_release_irq(&lock, flags);    // restores IRQ state
 *
 * The irq variants save EFLAGS before disabling interrupts and restore
 * them on release, so if interrupts were already off when you acquired
 * the lock, they stay off — no accidental re-enabling.
 */

typedef struct {
    volatile uint32_t locked; /* 0 = free, 1 = held */
} spinlock_t;

#define SPINLOCK_INIT { 0 }

void     spinlock_acquire(spinlock_t *lock);
void     spinlock_release(spinlock_t *lock);
void     spinlock_acquire_irq(spinlock_t *lock, uint32_t *flags);
void     spinlock_release_irq(spinlock_t *lock, uint32_t  flags);
int      spinlock_is_locked(const spinlock_t *lock);

#endif /* SPINLOCK_H */
