#include "inode.h"

/*NOTE: 同理，一个简单的实现，没考虑内存布局和字节序 */
size_t inode_bin_size(void)
{
    return sizeof(inode_t);
}

void inode_init(inode_t* inode)
{
    inode->mode = 0;
}
