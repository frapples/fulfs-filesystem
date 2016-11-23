
#include"../device_io.h"
#include "filesystem.h"
#include "inode.h"
#include "superblock.h"

#include "../utils/math.h"



fulfs_errcode_t fulfs_format(device_handle_t device, int sectors_per_block)
{
    /* int bytes_per_sector = 512; */
    /* block_no_t block_count = device_section_count(device) / sectors_per_block; */
    /* if (block_count  <= 0) { */
    /*     return FULFS_FAIL; */
    /* } */

    /* /\* inode 所占的block数 *\/ */
    /* int inode_block_count = count_groups(INODE_MAX_COUNT, sectors_per_block * bytes_per_sector / inode_bin_size()); */


    return FULFS_SUCCESS;
}
