#ifndef __FULFS_INODE__H__
#define __FULFS_INODE__H__

#include <stdint.h>
#include <stddef.h>

#include "def.h"
#include "block.h"
#include "superblock.h"


/* 如果INODE_MAX_COUNT是inode_no_t的最大值的话，uint16_t的 i < INODE_MAX_COUNT永真，所以为了方便减一 */
#define INODE_MAX_COUNT (UINT16_MAX - 1)

typedef struct {
    device_handle_t device;
    size_t block_size;
    block_no_t start;
    block_no_t size;
}dev_inode_ctrl_t;

#define LEVEL_0_INDIRECT_COUNT 10
typedef struct
{
    uint16_t mode;
    uint16_t link_count;
    uint64_t size;
    block_no_t blocks[LEVEL_0_INDIRECT_COUNT];
    block_no_t single_indirect_block;
    block_no_t double_indirect_block;
    block_no_t triple_indirect_block;
    timestamp_t accessed_time;
    timestamp_t modified_time;
    timestamp_t created_time;
}inode_t;

enum {
    MODE_EMPTY,
    MODE_FILE,
    MODE_DIR,
    MODE_SYMBOL_LINK
};

size_t inode_bin_size(void);


void inode_init(inode_t* inode);

void dev_inode_ctrl_init(dev_inode_ctrl_t* dev_inode_ctrl, device_handle_t device, size_t block_size, block_no_t start, block_no_t size);
void dev_inode_ctrl_init_from_superblock(dev_inode_ctrl_t* dev_inode_ctrl, device_handle_t device, const superblock_t* sb);


/* 读取no号inode */
bool inode_load(dev_inode_ctrl_t* dev_inode_ctrl, inode_no_t no, inode_t* inode);
/* 写入no号inode */
bool inode_dump(dev_inode_ctrl_t* dev_inode_ctrl, inode_no_t no, inode_t* inode);

int inode_block_count(size_t block_size, int inode_count);


bool inode_alloc(dev_inode_ctrl_t* dev_inode_ctrl, inode_no_t* p_inode_no); /* 分配一个空闲的inode */
void inode_free(dev_inode_ctrl_t* dev_inode_ctrl, inode_no_t no); /* 释放inode */

#endif /* __FULFS_INODE__H__ */
