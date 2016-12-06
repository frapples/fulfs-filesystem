#include "inode.h"

#include "../utils/math.h"
#include <assert.h>

/*NOTE: 同理，一个简单的实现，没考虑内存布局和字节序 */
size_t inode_bin_size(void)
{
    return sizeof(inode_t);
}

void inode_init(inode_t* inode)
{
    inode->mode = 0;
}


/* 把inode号转为block号和block内偏移 */
static void inode_no_to_block_no_and_offset(int sectors_per_block, inode_no_t no, block_no_t inode_table_start, block_no_t* p_block_no, size_t* p_offset)
{
    int inode_blocksize = count_groups(INODE_MAX_COUNT, sectors_per_block * BYTES_PER_SECTOR / inode_bin_size());
    /* TODO */
}

bool inode_load(device_handle_t device, block_no_t inode_table_start, inode_no_t no, inode_t* inode)
{
    /* TODO */
    assert(false);

    return true;
}

bool inode_dump(device_handle_t device, block_no_t inode_table_start, inode_no_t no, inode_t* inode)
{
    /* TODO */
    assert(false);

    return true;
}
