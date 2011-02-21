/*
 * Process startup.
 *
 * Copyright (C) 2003-2005 Juha Aatrokoski, Timo Lilja,
 *       Leena Salmela, Teemu Takanen, Aleksi Virtanen.
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
 * $Id: process.c,v 1.11 2007/03/07 18:12:00 ttakanen Exp $
 *
 */

#include "proc/process.h"
#include "proc/elf.h"
#include "proc/syscall.h"
#include "kernel/spinlock.h"
#include "kernel/thread.h"
#include "kernel/assert.h"
#include "kernel/interrupt.h"
#include "kernel/config.h"
#include "kernel/sleepq.h"
#include "fs/vfs.h"
#include "drivers/yams.h"
#include "vm/vm.h"
#include "vm/pagepool.h"
#include "lib/libc.h"


/** @name Process startup
 *
 * This module contains a function to start a userland process.
 */

/** Spinlock which must be hold when manipulating the process table */
spinlock_t process_table_slock;

/** The table containing all processes in the system, whether active or not. */
process_table_t process_table[CONFIG_MAX_PROCESSES];

gcd_t file_stdin, file_stdout, file_stderr;

/**
 * Initializes the process table, the process table spinlock,
 * sets up stdin, stdout and stderr and finally sets of the idle process
 */
void process_init(void) {
    device_t *dev;
    gcd_t *gcd;
    int n;

    /* Find system console (first tty) */
    dev = device_get(YAMS_TYPECODE_TTY, 0);
    KERNEL_ASSERT(dev != NULL);

    gcd = (gcd_t *)dev->generic_device;
    KERNEL_ASSERT(gcd != NULL);

    memcopy(sizeof(gcd_t), &file_stdin, gcd);
    memcopy(sizeof(gcd_t), &file_stdout, gcd);
    memcopy(sizeof(gcd_t), &file_stderr, gcd);

    file_stdin.write = NULL;
    file_stdout.read = NULL;
    file_stderr.read = NULL;

    /* Initializes spinlock */
    spinlock_reset(&process_table_slock);

    /* Sets all processes to free */
    for(n = 0; n < CONFIG_MAX_PROCESSES; n++)
        process_table[n].state = PROCESS_FREE;

    /* TODO: Create an idle-process containing the already-existing idle-thread. */
}

/**
 * Starts one userland process. The thread calling this function will
 * be used to run the process and will therefore never return from
 * this function. This function asserts that no errors occur in
 * process startup (the executable file exists and is a valid ecoff
 * file, enough memory is available, file operations succeed...).
 * Therefore this function is not suitable to allow startup of
 * arbitrary processes.
 *
 * @executable The name of the executable to be run in the userland
 * process
 */
void process_start(const char *executable)
{
    thread_table_t *my_entry;
    pagetable_t *pagetable;
    uint32_t phys_page;
    context_t user_context;
    uint32_t stack_bottom;
    elf_info_t elf;
    openfile_t file;

    int i;

    interrupt_status_t intr_status;

    my_entry = thread_get_current_thread_entry();

    /* If the pagetable of this thread is not NULL, we are trying to
       run a userland process for a second time in the same thread.
       This is not possible. */
    KERNEL_ASSERT(my_entry->pagetable == NULL);

    pagetable = vm_create_pagetable(thread_get_current_thread());
    KERNEL_ASSERT(pagetable != NULL);

    intr_status = _interrupt_disable();
    my_entry->pagetable = pagetable;
    _interrupt_set_state(intr_status);

    file = vfs_open((char *)executable);
    /* Make sure the file existed and was a valid ELF file */
    KERNEL_ASSERT(file >= 0);
    KERNEL_ASSERT(elf_parse_header(&elf, file));

    /* Trivial and naive sanity check for entry point: */
    KERNEL_ASSERT(elf.entry_point >= PAGE_SIZE);

    /* Calculate the number of pages needed by the whole process
       (including userland stack). Since we don't have proper tlb
       handling code, all these pages must fit into TLB. */
    KERNEL_ASSERT(elf.ro_pages + elf.rw_pages + CONFIG_USERLAND_STACK_SIZE
                  <= _tlb_get_maxindex() + 1);

    /* Allocate and map stack */
    for(i = 0; i < CONFIG_USERLAND_STACK_SIZE; i++) {
        phys_page = pagepool_get_phys_page();
        KERNEL_ASSERT(phys_page != 0);
        vm_map(my_entry->pagetable, phys_page, 
               (USERLAND_STACK_TOP & PAGE_SIZE_MASK) - i*PAGE_SIZE, 1);
    }

    /* Allocate and map pages for the segments. We assume that
       segments begin at page boundary. (The linker script in tests
       directory creates this kind of segments) */
    for(i = 0; i < (int)elf.ro_pages; i++) {
        phys_page = pagepool_get_phys_page();
        KERNEL_ASSERT(phys_page != 0);
        vm_map(my_entry->pagetable, phys_page, 
               elf.ro_vaddr + i*PAGE_SIZE, 1);
    }

    for(i = 0; i < (int)elf.rw_pages; i++) {
        phys_page = pagepool_get_phys_page();
        KERNEL_ASSERT(phys_page != 0);
        vm_map(my_entry->pagetable, phys_page, 
               elf.rw_vaddr + i*PAGE_SIZE, 1);
    }

    /* Put the mapped pages into TLB. Here we again assume that the
       pages fit into the TLB. After writing proper TLB exception
       handling this call should be skipped. */
    intr_status = _interrupt_disable();
    tlb_fill(my_entry->pagetable);
    _interrupt_set_state(intr_status);
    
    /* Now we may use the virtual addresses of the segments. */

    /* Zero the pages. */
    memoryset((void *)elf.ro_vaddr, 0, elf.ro_pages*PAGE_SIZE);
    memoryset((void *)elf.rw_vaddr, 0, elf.rw_pages*PAGE_SIZE);

    stack_bottom = (USERLAND_STACK_TOP & PAGE_SIZE_MASK) - 
        (CONFIG_USERLAND_STACK_SIZE-1)*PAGE_SIZE;
    memoryset((void *)stack_bottom, 0, CONFIG_USERLAND_STACK_SIZE*PAGE_SIZE);

    /* Copy segments */

    if (elf.ro_size > 0) {
        /* Make sure that the segment is in proper place. */
        KERNEL_ASSERT(elf.ro_vaddr >= PAGE_SIZE);
        KERNEL_ASSERT(vfs_seek(file, elf.ro_location) == VFS_OK);
        KERNEL_ASSERT(vfs_read(file, (void *)elf.ro_vaddr, elf.ro_size)
                      == (int)elf.ro_size);
    }

    if (elf.rw_size > 0) {
        /* Make sure that the segment is in proper place. */
        KERNEL_ASSERT(elf.rw_vaddr >= PAGE_SIZE);
        KERNEL_ASSERT(vfs_seek(file, elf.rw_location) == VFS_OK);
        KERNEL_ASSERT(vfs_read(file, (void *)elf.rw_vaddr, elf.rw_size)
                      == (int)elf.rw_size);
    }


    /* Set the dirty bit to zero (read-only) on read-only pages. */
    for(i = 0; i < (int)elf.ro_pages; i++) {
        vm_set_dirty(my_entry->pagetable, elf.ro_vaddr + i*PAGE_SIZE, 0);
    }

    /* Insert page mappings again to TLB to take read-only bits into use */
    intr_status = _interrupt_disable();
    tlb_fill(my_entry->pagetable);
    _interrupt_set_state(intr_status);

    /* Initialize the user context. (Status register is handled by
       thread_goto_userland) */
    memoryset(&user_context, 0, sizeof(user_context));
    user_context.cpu_regs[MIPS_REGISTER_SP] = USERLAND_STACK_TOP;
    user_context.pc = elf.entry_point;

    thread_goto_userland(&user_context);

    KERNEL_PANIC("thread_goto_userland failed.");
}

/**
 * Spawns a new thread in which it loads a new executable from disk.
 */
process_id_t process_spawn(const char *executable) {
    process_id_t process_id;
    process_table_t *process;
    TID_t spawned_thread;
    interrupt_status_t intr_status;

    if(strlen(executable) >= CONFIG_MAX_PROCESS_NAME)
        return SYSCALL_ILLEGAL_ARGUMENT;

    /* Acquires spinlock to change process table */
    intr_status = _interrupt_disable();
    spinlock_acquire(&process_table_slock);

    process_id = 17; /* TODO: get the process id a smarter way */
    process = &(process_table[process_id]);

    /* Initializes open files */
    memoryset(process->files, 0, sizeof(process->files));
    memcopy(sizeof(gcd_t), &process->files[FILEHANDLE_STDIN], &file_stdin); 
    memcopy(sizeof(gcd_t), &process->files[FILEHANDLE_STDOUT], &file_stdout); 
    memcopy(sizeof(gcd_t), &process->files[FILEHANDLE_STDERR], &file_stderr); 

    /* Sets process name and state */
    stringcopy(process->process_name, executable, CONFIG_MAX_PROCESS_NAME);
    process->state = PROCESS_ALIVE;

    /* Spawns the a new thread for the process */
    spawned_thread = thread_create((void (*)(uint32_t)) &process_start, (uint32_t) (process->process_name));
    thread_set_process_id(spawned_thread, process_id);
    thread_run(spawned_thread);

    /* Releases lock */
    spinlock_release(&process_table_slock);
    _interrupt_set_state(intr_status);

    return process_id;
}

/**
 * Frees a process' resources, kills its thread and sets it to a zombie-state,
 * so it can be finished by its parent.
 */
void process_finish(int retval) {
    interrupt_status_t intr_status;
    TID_t thread_id;
    thread_table_t *thread;
    process_table_t *process;

    /* Acquire the lock */
    intr_status = _interrupt_disable();
    spinlock_acquire(&process_table_slock);

    thread_id = thread_get_current_thread();
    thread = thread_get_current_thread_entry();
    process = &process_table[thread->process_id];

    if(process->state == PROCESS_ALIVE) {
        process->retval = retval;
        process->state = PROCESS_ZOMBIE;

        /* Kills thread */
        vm_destroy_pagetable(thread->pagetable);
        thread->pagetable = NULL;
        thread_finish();

        /* Wake up processes waiting to join */
        sleepq_wake_all(process);
    }

    spinlock_release(&process_table_slock);
    _interrupt_set_state(intr_status);

}

/**
 * Gets the current process id, by looking at the id from the currently running thread.
 */
process_id_t process_get_current_process(void) {
    return thread_get_current_thread_entry()->process_id;
}

/**
 * Gets the current row in the process table associated with the currently running thread,
 * by using process_get_current_process.
 */
process_table_t *process_get_current_process_entry(void) {
    return &process_table[process_get_current_process()];
}

/**
 * Waits for a given process to finish, then cleans up after it and
 * returns the process exitcode.
 */
int process_join(process_id_t pid) {
    process_table_t *process;
    interrupt_status_t intr_status;
    int retval;

    if(pid <= 0 || pid >= CONFIG_MAX_PROCESSES)
        return SYSCALL_ILLEGAL_ARGUMENT;

    /* Acquires lock to change the process table */
    intr_status = _interrupt_disable();
    spinlock_acquire(&process_table_slock);

    process = &process_table[pid];

    /* Called join on a non-existant process */
    if(process->state == PROCESS_FREE) {
        spinlock_release(&process_table_slock);
        _interrupt_set_state(intr_status);
        return SYSCALL_NOT_RUNNING;
    }

    /* If it is still runnig, wait for it to exit */
    if(process->state == PROCESS_ALIVE) {
        /* TODO: wait for exit */
    }

    /* Finishes up the process */
    retval = process->retval;
    process->state = PROCESS_FREE;

    /* Releases the lock */
    spinlock_release(&process_table_slock);
    _interrupt_set_state(intr_status);

    return retval;

}

/** @} */
