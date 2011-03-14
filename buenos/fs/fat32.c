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

int little_to_big_short(int);
int little_to_big_int(int);

typedef struct {
    /* Reference to the boot secter headers */
    fat32_BPP_struct *fat32_BPP;

    /* Pointer to gbd device performing fat32 */
    gbd_t          *disk;

    /* lock for mutual exclusion of fs-operations (we support only
       one operation at a time in any case) */
    semaphore_t    *lock;

    unsigned long fat_begin_lba;
    unsigned long cluster_begin_lba;
    unsigned char sectors_per_cluster;
    unsigned long root_dir_first_cluster;

} fat32_t;


fs_t * fat32_init(gbd_t *disk) {
    uint32_t addr;
    gbd_request_t req;
    fs_t *fs;
    fat32_t *fat32;
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
    KERNEL_ASSERT(PAGE_SIZE >= (3*FAT32_BLOCK_SIZE+sizeof(fat32_BPP_struct)+sizeof(fat32_t)+sizeof(fs_t)));

    /* Assert that the struct size is a multiple of 4 */
    KERNEL_ASSERT(sizeof(fat32_BPP_struct) % 4 == 0);
    
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
    fat32 = (void *) (addr + sizeof(fat32_BPP_struct));
    fs = (fs_t *) (addr + sizeof(fat32_BPP_struct) + sizeof(fat32_t *));

    /* save the semaphore to the fat32_t */
    fat32->lock = sem;

    fat32->disk = disk;
    fat32->fat32_BPP = fat32_BPP;
    fat32->fat_begin_lba = little_to_big_short(fat32_BPP->BPB_RsvdSecCnt);
    fat32->cluster_begin_lba =
        little_to_big_short(fat32_BPP->BPB_RsvdSecCnt) +
        (fat32_BPP->BPB_NumFATs * little_to_big_int(fat32_BPP->BPB_FATSz32));
    fat32->sectors_per_cluster = fat32_BPP->BPB_SecPerClus;
    fat32->root_dir_first_cluster = little_to_big_int(fat32_BPP->BPB_RootClus);

    kprintf("fat_begin_lba: %x\n", fat32->fat_begin_lba);
    kprintf("cluster_begin_lba: %x\n", fat32->cluster_begin_lba);
    kprintf("sectors_per_cluster: %x\n", fat32->sectors_per_cluster);
    kprintf("root_dir_first_cluster: %x\n", fat32->root_dir_first_cluster);

    fs->internal = (void *)fat32;

    stringcopy(fs->volume_name, fat32_BPP->BS_VolLab, sizeof(fat32_BPP->BS_VolLab));

    fs->unmount = fat32_unmount;
    fs->open    = fat32_open;
    fs->close   = fat32_close;
    fs->create  = fat32_create;
    fs->remove  = fat32_remove;
    fs->read    = fat32_read;
    fs->write   = fat32_write;
    fs->getfree  = fat32_getfree;

    return fs;
}

int little_to_big_short(int x) {
    int result = 0;

    result |= (x >> 8) & 0x00ff;
    result |= (x << 8) & 0xff00;

    return result;

}

int little_to_big_int(int x) {
    int result = 0;

    result |= little_to_big_short(x) << 16;
    result |= little_to_big_short(x >> 16);

    return result;
}

int fat32_unmount(fs_t *fs) {
    fat32_t *fat32;

    fat32 = (fat32_t *)fs->internal;

    semaphore_P(fat32->lock); /* The semaphore should be free at this
      point, we get it just in case something has gone wrong. */

    /* free semaphore and allocated memory */
    semaphore_destroy(fat32->lock);
    pagepool_free_phys_page(ADDR_KERNEL_TO_PHYS((uint32_t)fs));
    return VFS_OK;
}

int fat32_open(fs_t *fs, char *filename) {
    fs = fs;
    filename = filename;
    return 0;
}

int fat32_close(fs_t *fs, int fileid) {
    fs = fs;
    fileid = fileid;

    return 0;
}

int fat32_create(fs_t *fs, char *filename, int size) {
    fs = fs;
    filename = filename;
    size = size;

    return 0;
}

int fat32_remove(fs_t *fs, char *filename) {
    fs = fs;
    filename = filename;

    return 0;
}

int fat32_read(fs_t *fs, int fileid, void *buffer, int bufsize, int offset) {
    fs = fs;
    fileid = fileid;
    buffer = buffer;
    bufsize = bufsize;
    offset = offset;

    return 0;
}
int fat32_write(fs_t *fs, int fileid, void *buffer, int datasize, int offset) {
    fs = fs;
    fileid = fileid;
    buffer = buffer;
    datasize = datasize;
    offset = offset;

    return 0;
}

int fat32_getfree(fs_t *fs) {
    fs = fs;

    return 0;
}
