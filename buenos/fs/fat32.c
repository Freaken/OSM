/*
 * FAT32 (File Allocation Table)
 *
 * Copyright (C) 2011 The noobs
 */

typedef struct {
    /* Total number of blocks of the disk */
    uint32_t totalblocks;

    /* lock for mutual exclusion of fs-operations (we support only
       one operation at a time in any case) */
    semaphore_t    *lock;

} fat32_t;


fs_t * fat32_init(gbd_t *disk) {

    uint32_t addr;
    gbd_request_t req;
    fs_t *fs;
    fat32_t *fat32;
    int r;
    semaphore_t *sem;

    if(disk->block_size(disk) != TFS_BLOCK_SIZE)
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

    if(((uint32_t *)addr)[0] != TFS_MAGIC) {
        semaphore_destroy(sem);
        pagepool_free_phys_page(ADDR_KERNEL_TO_PHYS(addr));
        return NULL;
    }

    /* fs_t, fat32_t and all buffers in fat32_t fit in one page, so obtain
       addresses for each structure and buffer inside the allocated
       memory page. */
    fs  = (fs_t *)addr;
    fat32 = (fat32_t *)(addr + sizeof(fs_t));
    fat32->buffer_inode = (fat32_inode_t *)((uint32_t)fat32 + sizeof(fat32_t));
    fat32->buffer_bat  = (bitmap_t *)((uint32_t)fat32->buffer_inode + 
                                    TFS_BLOCK_SIZE);
    fat32->buffer_md   = (fat32_direntry_t *)((uint32_t)fat32->buffer_bat + 
                                        TFS_BLOCK_SIZE);

    fat32->totalblocks = MIN(disk->total_blocks(disk), 8*TFS_BLOCK_SIZE);
    fat32->disk        = disk;

    /* save the semaphore to the fat32_t */
    fat32->lock = sem;

    fs->internal = (void *)fat32;
    stringcopy(fs->volume_name, name, VFS_NAME_LENGTH);

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

