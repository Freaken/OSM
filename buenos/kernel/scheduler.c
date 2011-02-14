/*
 * Scheduler.
 *
 * Copyright (C) 2003 Juha Aatrokoski, Timo Lilja,
 *   Leena Salmela, Teemu Takanen, Aleksi Virtanen.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: scheduler.c,v 1.14 2007/02/25 15:16:29 jaatroko Exp $
 *
 */

#include "kernel/thread.h"
#include "kernel/spinlock.h"
#include "kernel/assert.h"
#include "kernel/panic.h"
#include "kernel/interrupt.h"
#include "lib/libc.h"
#include "kernel/config.h"
#include "drivers/timer.h"

/** @name Scheduler
 *
 * This module implements simple round robin scheduler.
 *
 */

/* Import thread table and its lock from thread.c */
extern spinlock_t thread_table_slock;
extern thread_table_t thread_table[CONFIG_MAX_THREADS];

/** Currently running thread on each CPU */
TID_t scheduler_current_thread[CONFIG_MAX_CPUS];

/** List of threads ready to be run. */
static struct {
    TID_t head; /* the first thread in ready to run queue, negative if none */
    TID_t tail; /* the last thread in ready to run queue, negative if none */
} scheduler_ready_to_run = {-1, -1};

/**
 * Initializes the scheduler current thread table to 0 for each processor.
 */
void scheduler_init(void) {
    int i;
    for (i=0; i<CONFIG_MAX_CPUS; i++)
	scheduler_current_thread[i] = 0;
}

/**
 * Adds given thread to scheduler's ready to run list. Doesn't do 
 * any synchronization, it is assumed that spinlock to the thread table
 * is held and interrups are disabled when calling this function.
 * 
 * @param t thread to add to ready list
 *
 */

void scheduler_add_to_ready_list(TID_t t)
{
    /* Idle thread should never go into the ready list */
    KERNEL_ASSERT(t != IDLE_THREAD_TID);

    /* Sanity check */
    KERNEL_ASSERT(t >= 0 && t < CONFIG_MAX_THREADS);

    if (scheduler_ready_to_run.tail < 0) {
	/* ready queue was empty */
	scheduler_ready_to_run.head = t;
	scheduler_ready_to_run.tail = t;
	thread_table[t].next = -1;
    } else {
	/* ready queue was not empty */
	thread_table[scheduler_ready_to_run.tail].next = t;
	thread_table[t].next = -1;
	scheduler_ready_to_run.tail = t;
    }
}

/**
 * Removes the first thread from the ready to run list and returns it.
 * if the list was empty, returns the idle thread (TID 0). It is assumed
 * that interrupts are disabled and thread table spinlock is held when
 * this function is called.
 *
 * @return The removed thread.
 *
 */

static TID_t scheduler_remove_first_ready(void)
{
    TID_t t;

    t = scheduler_ready_to_run.head;

    /* Idle thread should never be on the ready list. */
    KERNEL_ASSERT(t != IDLE_THREAD_TID);

    if(t >= 0) {
        /* Threads in ready queue should be in state Ready */
        KERNEL_ASSERT(thread_table[t].state == THREAD_READY);
	if(scheduler_ready_to_run.tail == t) {
	    scheduler_ready_to_run.tail = -1;
	}
	scheduler_ready_to_run.head =
	    thread_table[scheduler_ready_to_run.head].next;

    }

    if(t < 0) {
	return IDLE_THREAD_TID;
    } else {
	return t;
    }
}

/**
 * Adds given thread to scheduler's ready to run list. This function
 * handles syncronization and can be called from anywhere where
 * needed. Must not be called if thread table spinlock is already held.
 *
 * @param t Thread to add. The thread must not already be on the ready
 * list or running.
 * 
 */

void scheduler_add_ready(TID_t t)
{
    interrupt_status_t intr_status;
    
    intr_status = _interrupt_disable();

    spinlock_acquire(&thread_table_slock);

    scheduler_add_to_ready_list(t);
    thread_table[t].state = THREAD_READY;

    spinlock_release(&thread_table_slock);

    _interrupt_set_state(intr_status);
}


/**
 * Select next thread for running. Removes the currently running
 * thread running on this CPU and selects new running thread.
 * Circulates threads in round robin manner. Must be called only from
 * interrupt/exception handlers and code assumes that interrupts are
 * disabled (which is the case in interrupt handlers).
 *
 * Scheduler also handles thread table row freeing when thread is
 * DYING and removes threads wishing to sleep (sleeps_on != 0) from
 * ready status and places them SLEEPING. Syncronizes access to thread
 * table by acquiring the thread table spinlock.
 *
 * After selecting new thread for running the scheduler will reset the
 * CP0 timer to cause timer interrupt after thread's timeslice is
 * over.
 *
 */

void scheduler_schedule(void)
{
    TID_t t;
    thread_table_t *current_thread;
    int this_cpu;

    this_cpu = _interrupt_getcpu();

    spinlock_acquire(&thread_table_slock);

    current_thread = &(thread_table[scheduler_current_thread[this_cpu]]);

    if(current_thread->state == THREAD_DYING) {
	current_thread->state = THREAD_FREE;
    } else if(current_thread->sleeps_on != 0) {
	current_thread->state = THREAD_SLEEPING;
    } else {
	if(scheduler_current_thread[this_cpu] != IDLE_THREAD_TID)
	    scheduler_add_to_ready_list(scheduler_current_thread[this_cpu]);
	current_thread->state = THREAD_READY;
    }

    t = scheduler_remove_first_ready();
    thread_table[t].state = THREAD_RUNNING;

    spinlock_release(&thread_table_slock);

    scheduler_current_thread[this_cpu] = t;

    /* Schedule timer interrupt to occur after thread timeslice is spent */
    timer_set_ticks(_get_rand(CONFIG_SCHEDULER_TIMESLICE) + 
                    CONFIG_SCHEDULER_TIMESLICE / 2);
}
