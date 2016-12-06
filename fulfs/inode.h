#ifndef __FULFS_INODE__H__
#define __FULFS_INODE__H__

#include <stdint.h>
#include <stddef.h>

#include "block.h"


/* 如果INODE_MAX_COUNT是inode_no_t的最大值的话，uint16_t的 i < INODE_MAX_COUNT永真，所以为了方便减一 */
#define INODE_MAX_COUNT (UINT16_MAX - 1)
typedef uint16_t inode_no_t;

typedef uint64_t time_t;

typedef struct
{
    uint16_t mode;
    uint16_t link_count;
    uint64_t size;
    block_no_t blocks[10];
    block_no_t single_indirect_block;
    block_no_t double_indirect_block;
    block_no_t triple_indirect_block;
    time_t accessed_time;
    time_t modified_time;
    time_t created_time;
}inode_t;

size_t inode_bin_size(void);


void inode_init(inode_t* inode);


/* 读取no号inode */
bool inode_load(device_handle_t device,int sectors_per_block, block_no_t inode_table_start, inode_no_t no, inode_t* inode);
/* 写入no号inode */
bool inode_dump(device_handle_t device,int sectors_per_block, block_no_t inode_table_start, inode_no_t no, inode_t* inode);

int inode_block_count(int sectors_per_block, int inode_count);

#endif /* __FULFS_INODE__H__ */
