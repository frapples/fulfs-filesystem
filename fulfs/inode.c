#include "inode.h"

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
