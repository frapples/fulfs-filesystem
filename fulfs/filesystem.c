#include "filesystem.h"
#include "block.h"
#include "inode.h"
#include "superblock.h"
#include "data_block.h"

#include "../utils/math.h"


fulfs_errcode_t fulfs_format(device_handle_t device, int sectors_per_block)
{
    block_no_t block_count = device_section_count(device) / sectors_per_block;
    block_no_t inode_table = 1;

    /* inode 所占的block数 */
    int inode_blocksize =  inode_block_count(sectors_per_block * BYTES_PER_SECTOR, INODE_MAX_COUNT);

    block_no_t data_block = inode_table + inode_blocksize;


    if (block_count  <= 0 ||
        data_block >= block_count) {
        return FULFS_FAIL;
    }


    /* 初始化磁盘的inode区 */
    dev_inode_ctrl_t dev_inode_ctrl;
    dev_inode_ctrl_init(&dev_inode_ctrl, device, sectors_per_block * BYTES_PER_SECTOR, inode_table, inode_blocksize);

    inode_t inode;
    inode_init(&inode);

    for (inode_no_t i = 0; i < INODE_MAX_COUNT; i++) {
        if (inode_dump(&dev_inode_ctrl, i, &inode)) {
            return FULFS_FAIL;
        }
    }

    /* 初始化磁盘的data block区 */
    block_no_t data_block_free_stack = data_blocks_init(device, sectors_per_block, data_block, block_count - data_block);

    superblock_t sb;
    superblock_create(&sb, device_section_count(device), sectors_per_block,
                      inode_table, data_block, data_block_free_stack);

    /* 写入superblock */
    if (superblock_dump(device, &sb)) {
        return FULFS_FAIL;
    }

    return FULFS_SUCCESS;
}
