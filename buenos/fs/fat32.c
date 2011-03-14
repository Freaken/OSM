/*
 * FAT32 (File Allocation Table)
 *
 * Copyright (C) 2011 Ye Olde Overlords
 */

#include "drivers/gbd.h"
#include "fs/fat32.h"
#include "kernel/assert.h"
#include "kernel/semaphore.h"
#include "vm/pagepool.h"

typedef struct {
    /* Reference to the boot secter headers */
    fat32_BPP_struct *fat32_BPP;

    /* Pointer to gbd device performing tfs */
    gbd_t          *disk;

    /* lock for mutual exclusion of fs-operations (we support only
       one operation at a time in any case) */
    semaphore_t    *lock;

} fat32_t;


fs_t * fat32_init(gbd_t *disk) {
    uint32_t addr;
    gbd_request_t req;
    /*fs_t *fs;
    fat32_t *fat32;*/
    fat32_BPP_struct *fat32_BPP;
    int r;
    semaphore_t *sem;

    if(disk->block_size(disk) != FAT32_BLOCK_SIZE)
        return NULL;

    /* check semaphore availability before memory allocation */
    sem = semaphore_create(1);
    if (sem == NULL) {
        kprintf("fat32_init: could not create a new semaphore.\n");
        return NULL;
    }

    addr = pagepool_get_phys_page();
    if(addr == 0) {
        semaphore_destroy(sem);
        kprintf("fat32_init: could not allocate memory.\n");
        return NULL;
    }
    addr = ADDR_PHYS_TO_KERNEL(addr);      /* transform to vm address */


    /* Assert that one page is enough */
    KERNEL_ASSERT(PAGE_SIZE >= (3*FAT32_BLOCK_SIZE+sizeof(fat32_t)+sizeof(fs_t)));
    
    /* Read header block, and make sure this is fat32 drive */
    req.block = 0;
    req.sem = NULL;
    req.buf = ADDR_KERNEL_TO_PHYS(addr);   /* disk needs physical addr */
    r = disk->read_block(disk, &req);
    if(r == 0) {
        semaphore_destroy(sem);
        pagepool_free_phys_page(ADDR_KERNEL_TO_PHYS(addr));
        kprintf("fat32_init: Error during disk read. Initialization failed.\n");
        return NULL; 
    }
    short *fat32_magic = (short*) (addr + 510);

    if(*fat32_magic != 0x55aa) {
        semaphore_destroy(sem);
        pagepool_free_phys_page(ADDR_KERNEL_TO_PHYS(addr));
        return NULL;
    }

    fat32_BPP  = (fat32_BPP_struct *)addr;
    fat32 = (fat32_t *) (addr + sizeof(fat32_BPP_struct));
    fs = (fs_t *) (addr + sizeof(fat32_BPP_struct) + sizeof(fat32_t *));

    /* save the semaphore to the fat32_t */
    fat32->disk = disk;
    fat32->lock = sem;
    fat32->fat32_BPP = fat32_BPP;

    fs->internal = (void *)fat32;

    /* TODO: Fill out volume name */

    fs->unmount = fat32_unmount;
    fs->open    = fat32_open;
    fs->close   = fat32_close;
    fs->create  = fat32_create;
    fs->remove  = fat32_remove;
    fs->read    = fat32_read;
    fs->write   = fat32_write;
    fs->getfree  = fat32_getfree;

    return NULL;
}

/*
int fat32_unmount(fs_t *fs)
{return 0;}
int fat32_open(fs_t *fs, char *filename)
{return 0;}
int fat32_close(fs_t *fs, int fileid)
{return 0;}
int fat32_create(fs_t *fs, char *filename, int size)
{return 0;}
int fat32_remove(fs_t *fs, char *filename)
{return 0;}
int fat32_read(fs_t *fs, int fileid, void *buffer, int bufsize, int offset)
{return 0;}
int fat32_write(fs_t *fs, int fileid, void *buffer, int datasize, int offset)
{return 0;}
int fat32_getfree(fs_t *fs)
{return 0;}
*/
