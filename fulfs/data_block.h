#ifndef __FULFS_DATA_BLOCK__H__
#define __FULFS_DATA_BLOCK__H__

#include "block.h"
#include "../device_io.h"

block_no_t data_blocks_init(device_handle_t device, int sectors_per_block, block_no_t data_block_start, block_no_t data_block_count);


#endif /* __FULFS_DATA_BLOCK__H__ */
