#include "kernel/spinlock.h"
#include "kernel/thread.h"

typedef struct {
    spinlock_t slock;
    bool locked;
} lock_t;

int lock_reset(lock_t *lock);
void lock_acquire(lock_t *lock);
void lock_release(lock_t *lock);

typedef struct { } cond_t;

int condition_reset(cond_t *cond);
void condition_wait(cond_t *cond, lock_t *condition_lock);
void condition_signal(cond_t *cond);
void condition_broadcast(cond_t *cond);
