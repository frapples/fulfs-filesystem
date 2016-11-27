#include "filesystem.h"
#include "block.h"
#include "inode.h"
#include "superblock.h"
#include "data_block.h"

#include "../utils/math.h"


fulfs_errcode_t fulfs_format(device_handle_t device, int sectors_per_block)
{
    int bytes_per_sector = 512;
    block_no_t block_count = device_section_count(device) / sectors_per_block;
    block_no_t inode_table = 1;

    /* inode 所占的block数 */
    int inode_blocksize = count_groups(INODE_MAX_COUNT, sectors_per_block * bytes_per_sector / inode_bin_size());

    block_no_t data_block = inode_table + inode_blocksize;


    if (block_count  <= 0 ||
        data_block >= block_count) {
        return FULFS_FAIL;
    }


    /* 初始化磁盘的inode区 */
    inode_t inode;
    inode_init(&inode);

    for (inode_no_t i = 0; i < INODE_MAX_COUNT; i++) {
        if (inode_dump(device, inode_table, i, &inode)) {
            return FULFS_FAIL;
        }
    }

    /* 初始化磁盘的data block区 */
    block_no_t data_block_free_stack = data_blocks_init(device, data_block, block_count - data_block, sectors_per_block);

    superblock_t sb;
    superblock_create(&sb, device_section_count(device), sectors_per_block,
                      inode_table, data_block, data_block_free_stack);

    /* 写入superblock */
    if (superblock_dump(device, &sb)) {
        return FULFS_FAIL;
    }

    return FULFS_SUCCESS;
}
