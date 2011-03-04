#include "kernel/spinlock.h"
#include "kernel/thread.h"

typedef struct {
    spinlock_t slock;
    bool locked;
} lock_t;

int lock_reset(lock_t *lock);
void lock_acquire(lock_t *lock);
void lock_release(lock_t *lock);
