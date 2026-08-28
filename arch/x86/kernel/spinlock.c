#include "spinlock.h"
#include <stdint.h>

void spinlock_acquire(spinlock_t *lock) {
  while (__sync_lock_test_and_set(&lock->locked, 1)) {
    while (lock->locked)
      asm volatile("pause");
  }

  __sync_synchronize();
}

void spinlock_release(spinlock_t *lock) { __sync_lock_release(&lock->locked); }

void spinlock_acquire_irq(spinlock_t *lock, uint32_t *flags) {
  asm volatile("pushf\n"
               "pop %0\n"
               "cli\n"
               : "=r"(*flags)
               :
               : "memory");

  spinlock_acquire(lock);
}

void spinlock_release_irq(spinlock_t *lock, uint32_t flags) {
  spinlock_release(lock);

  asm volatile("push %0\n"
               "popf\n"
               :
               : "r"(flags)
               : "memory");
}

int spinlock_is_locked(const spinlock_t *lock) { return lock->locked != 0; }
