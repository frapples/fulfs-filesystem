#ifndef __FULFS_INODE__H__
#define __FULFS_INODE__H__

#include <stdint.h>
#include <stddef.h>

#include "block.h"

#define INODE_MAX_COUNT (2 << (16 - 1))
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

#endif /* __FULFS_INODE__H__ */
