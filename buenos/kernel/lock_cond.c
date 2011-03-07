#include "kernel/interrupt.h"
#include "lock_cond.h"
#include "sleepq.h"

/** Initializes lock for use. Should always be called
  * before use! Makes the internal state valid. */
int lock_reset(lock_t *lock) {
    spinlock_reset(&lock->slock);
    lock->locked = 0;

    return 0;
}

/** Acquires the lock. If the lock is already
  * held by somebody else puts the thread to
  * sleep until the lock is available. */
void lock_acquire(lock_t *lock) {
    interrupt_status_t intr_status;

    intr_status = _interrupt_disable();
    spinlock_acquire(&lock->slock);

    while(lock->locked) {
        sleepq_add(lock);
        spinlock_release(&lock->slock);
        thread_switch();
        spinlock_acquire(&lock->slock);
    }

    lock->locked = 1;
    spinlock_release(&lock->slock);

    _interrupt_set_state(intr_status);
}

/** Releases the lock. Wakes up any one thread
  * waiting to acquire the lock */
void lock_release(lock_t *lock) {
    interrupt_status_t intr_status;

    intr_status = _interrupt_disable();
    spinlock_acquire(&lock->slock);

    lock->locked = 0;
    sleepq_wake(lock);

    spinlock_release(&lock->slock);
    _interrupt_set_state(intr_status);
}

/** Useless! Literally no use.
  * Should still be used to 'initialize' condition
  * in code in case structure changes in the future. */
int condition_reset(cond_t *cond) {
    /* Dummy statement to make gcc happy */
    cond = cond;

    return 0;
}

/** Wait for condition. Puts the thread to sleep until the
  * condition is satisfied as signalled by another thread.
  * Releases the condition lock before going to sleep and
  * re-acquires it upon awakening. Uses mesa semantics and
  * programmer should ALWAYS recheck that the condition is
  * still true when the thread is awoken, usually by encap-
  * sulating a call to condition_wait in a while(!condition)
  * loop. */
void condition_wait(cond_t *cond, lock_t *condition_lock) {
    interrupt_status_t intr_status;
    intr_status = _interrupt_disable();
    
    sleepq_add(cond);
    lock_release(condition_lock);
    thread_switch();
    lock_acquire(condition_lock);

    _interrupt_set_state(intr_status);
}

/** Signals that a given condition is now valid, waking up
  * any one thread waiting for this. Does not block the
  * signalling thread. Programmer should make sure that the
  * condition is true before signalling this. */
void condition_signal(cond_t *cond) {
    sleepq_wake(cond);
}

/** Same as signal, but wakes up ALL threads waiting on the
  * condition. */
void condition_broadcast(cond_t *cond) {
    sleepq_wake_all(cond);
}
