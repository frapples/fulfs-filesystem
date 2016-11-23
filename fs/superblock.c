#include "superblock.h"

#include <assert.h>
#include <string.h>


size_t superblock_bin_size(void)
{
    assert(sizeof(superblock_t) <= 512);

    return sizeof(superblock_t);
}

/* NOTE:目前的实现并未考虑太多的通用性，包括内存布局和字节序 */
void superblock_dump(superblock_t* sb, char* bin, size_t bin_size)
{
    assert(bin_size >= superblock_bin_size());

    *sb = *(superblock_t *)bin;
    memset(bin + superblock_bin_size(), 0, bin_size - superblock_bin_size());
}

void superblock_load(const char* bin, superblock_t* sb)
{
    *(superblock_t *)bin = *sb;
}

