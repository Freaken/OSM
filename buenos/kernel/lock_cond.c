#include "kernel/interrupt.h"
#include "lock_cond.h"
#include "sleepq.h"

int lock_reset(lock_t *lock) {
    spinlock_reset(&lock->slock);
    lock->locked = false;

    return 0;
}

void lock_acquire(lock_t *lock) {
    interrupt_status_t intr_status;

    intr_status = _interrupt_disable();
    spinlock_acquire(&lock->slock);

    if(lock->locked) {
        sleepq_add(lock);
        spinlock_release(&lock->slock);
        thread_switch();
    } else {
        lock->locked = true;
        spinlock_release(&lock-slock);
    }
    _interrupt_set_state(intr_status);
}

void lock_release(lock_t *lock) {
    interrupt_status_t intr_status;

    intr_status = _interrupt_disable();
    spinlock_acquire(&lock->slock);

    lock->locked = false;
    sleepq_wake(lock);

    spinlock_release(&lock->slock);
    _interrupt_set_state(intr_status);
}
