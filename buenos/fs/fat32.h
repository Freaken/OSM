/*
 * FAT32 (File Allocation Table)
 *
 * Copyright (C) 2011 The noobs
 */

#ifndef FS_FAT32_H
#define FS_FAT32_H

#include "fs/filesystems.h"

/* FAT32 sector size */
#define FAT32_BLOCK_SIZE 512

/* FAT32 magic */
#define FAT32_MAGIC 0x55aa


/* Boot sector information */

struct fat32_BPP_struct {              /* Offset | Size */
    char BS_jmpBoot[3];                /*      0 |    3 */
    char BS_OEMName[8];                /*      3 |    8 */
    unsigned short BPB_BytsPerSec;     /*     11 |    2 */
    unsigned char  BPB_SecPerClus;     /*     13 |    1 */
    unsigned short BPB_RsvdSecCnt;     /*     14 |    2 */
    unsigned char  BPB_NumFATs;        /*     16 |    1 */
    unsigned short BPB_RootEntCnt;     /*     17 |    2 */
    unsigned short BPB_TotSec16;       /*     19 |    2 */
    unsigned char  BPB_Media;          /*     21 |    1 */
    unsigned short BPB_FATSz16;        /*     22 |    2 */
    unsigned short BPB_SecPerTrk;      /*     24 |    2 */
    unsigned short BPB_NumHeads;       /*     26 |    2 */
    unsigned int   BPB_HiddSec;        /*     28 |    4 */
    unsigned int   BPB_TotSec32;       /*     32 |    4 */
    unsigned int   BPB_FATSz32;        /*     36 |    4 */
    unsigned short BPB_ExtFlags;       /*     40 |    2 */
    unsigned short BPB_FSVer;          /*     42 |    2 */
    unsigned int   BPB_RootClus;       /*     44 |    4 */
    unsigned short BPB_FSInfo;         /*     48 |    2 */
    unsigned short BPB_BkBootSec;      /*     50 |    2 */
    unsigned char  BPB_Reserved[12];   /*     52 |   12 */
    unsigned char  BS_DrvNum;          /*     64 |    1 */
    unsigned char  BS_Reserved1;       /*     65 |    1 */
    unsigned char  BS_BootSig;         /*     66 |    1 */
    unsigned int   BS_VolID;           /*     67 |    4 */
    unsigned char  BS_VolLab[11];      /*     71 |   11 */
    unsigned char  BS_FilSysType[8];   /*     82 |    8 */
} __attribute__((__packed__)); 

typedef struct fat32_BPP_struct fat32_BPP_struct;


/* functions */
fs_t * fat32_init(gbd_t *disk);

int fat32_unmount(fs_t *fs);
int fat32_open(fs_t *fs, char *filename);
int fat32_close(fs_t *fs, int fileid);
int fat32_create(fs_t *fs, char *filename, int size);
int fat32_remove(fs_t *fs, char *filename);
int fat32_read(fs_t *fs, int fileid, void *buffer, int bufsize, int offset);
int fat32_write(fs_t *fs, int fileid, void *buffer, int datasize, int offset);
int fat32_getfree(fs_t *fs);


#endif

