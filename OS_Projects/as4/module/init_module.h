//
// Created by DouZhi on 16/5/18.
//

# ifndef _Ramdisk_
# define _Ramdisk_

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/utsname.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/errno.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/string.h>
#include <asm/unistd.h>
#include "../usr/filesys_ioctl.h"

# define FINE 0
# define ERROR -1

# define ramdisk_size 2097152	// 2MB, 8192 blocks
# define block_size 256			// in bytes
# define superblock_size 1		// in block
# define inode_list_size 256	// in block
# define inode_list_num 1024	// max number of file
# define bitmap_size 4			// in block
# define bitmap_sublock 1024	// bitmap sublock
# define allocatable_space_size 7931 // in block
# define max_file_size 1067008	// the maximum size of a file
# define dir "dir"
# define reg "reg"
# define nul "nul"

typedef struct
{
    uint16_t vfreeBlock;		// set to a number of available allocated space
    uint16_t vfreeInode;
    uint16_t first_vfree_bitmap_block;	// if = ERROR means
    uint16_t first_vfree_inode;
}superblock_t;

typedef struct
{
    char type[4];			// 4 bytes
    uint32_t size;			// 4 bytes
    void *location[10];		// 40 bytes
    uint16_t protaction;	// permission
    uint16_t operation;		// opertions could do on the file r,w,e
    uint16_t parent_index;	// the parent dirctory inode index
    // 4 bytes to store the write read pointer
    // offset[0]: 0-7 direct block if 8 means direct block full
    // 8-71 single_indirect block; 72-4176 double_indirect block
    // offset[1]: offset inside block
    int offset[2];
    uint16_t unused[4];		// 8 bytes unused
}inode_t;

typedef struct
{
    char filename[14];		// 14 bytes for file entry
    uint16_t inode_index;	// 2 bytes for inode index
}dirctory_entry_t;


typedef struct
{
    uint8_t index[bitmap_sublock];
}bitmap_t;

# endif