#include "superblock.h"

#include <assert.h>
#include <string.h>


size_t superblock_bin_size(void)
{
    assert(sizeof(superblock_t) <= 512);

    return sizeof(superblock_t);
}

/* NOTE:目前的实现并未考虑太多的通用性，包括内存布局和字节序 */
void superblock_dump_to_bin(superblock_t* sb, char* bin, size_t bin_size)
{
    assert(bin_size >= superblock_bin_size());

    *(superblock_t *)bin = *sb;
    memset(bin + superblock_bin_size(), 0, bin_size - superblock_bin_size());
}

void superblock_load_from_bin(const char* bin, superblock_t* sb)
{
    *sb = *(superblock_t *)bin;
}

void superblock_create(superblock_t* sb, sector_no_t sectors, int sectors_per_block,
                       block_no_t inode_table, inode_no_t inode_count, block_no_t data_block, block_no_t data_block_free_stack, inode_no_t root_inode)
{
    sb->sectors = sectors;
    sb->sectors_per_block = sectors_per_block;

    sb->total_size = ((sb->sectors / sb->sectors_per_block) - data_block) * (BYTES_PER_SECTOR * sb->sectors_per_block);
    /* sb->used_size = 0; */

    sb->root_dir = root_inode;
    sb->inode_table_block = inode_table;
    sb->data_block = data_block;
    sb->data_block_free_stack = data_block_free_stack;


    sb->used_inode_count = 1;
    sb->used_data_block_count = 0;
    sb->inode_total_count = inode_count;
}


bool superblock_load(device_handle_t device, superblock_t* sb)
{
    char buf[512];
    bool success = DEVICE_IO_SUCCESS(device_read(device, 0, 1, buf));
    if (!success) {
        return false;
    }
    superblock_load_from_bin(buf, sb);

    return true;
}

bool superblock_dump(device_handle_t device, superblock_t* sb)
{
    char buf[512];
    superblock_dump_to_bin(sb, buf, sizeof(buf));
    return DEVICE_IO_SUCCESS(device_write(device, 0, 1, buf));
}


block_no_t superblock_block_count(const superblock_t* sb)
{
    return sb->sectors / sb->sectors_per_block;
}

uint64_t superblock_total_size(const superblock_t* sb)
{
    return sb->total_size;
}
uint64_t superblock_used_size(const superblock_t* sb)
{
    fsize_t inode_table_total_size = superblock_inode_table_blocksize(sb) * superblock_block_size(sb);

    /* 按比例计算 */
    /* sb->used_inode_count / sb->inode_total_count * inode_table_total_size */
    fsize_t inode_used_size = sb->used_inode_count  * inode_table_total_size / sb->inode_total_count;
    fsize_t data_block_used_size = sb->used_data_block_count * superblock_block_size(sb);

    return inode_used_size + data_block_used_size;
}

uint64_t superblock_free_size(const superblock_t* sb)
{
    uint64_t used_size = superblock_used_size(sb);
    uint64_t total_size = superblock_total_size(sb);

    assert(used_size <= total_size);

    return total_size - used_size;
}

void superblock_set_used_size(superblock_t* sb, uint64_t new_size)
{
    assert(false);
    /* assert(sb->used_size + new_size <= sb->total_size); */

    /* sb->used_size = new_size; */

}

inode_no_t superblock_root_dir_inode(const superblock_t* sb)
{
    return sb->root_dir;
}


block_no_t superblock_inode_table_start(const superblock_t* sb)
{
    return sb->inode_table_block;
}

block_no_t superblock_inode_table_blocksize(const superblock_t* sb)
{
    assert(sb->data_block > sb->inode_table_block);

    return sb->data_block - sb->inode_table_block;
}

block_no_t superblock_data_block_start(const superblock_t* sb)
{
    return sb->data_block;
}

block_no_t superblock_data_block_size(const superblock_t* sb)
{
    assert(superblock_block_count(sb) > sb->data_block);

    return superblock_block_count(sb) - sb->data_block;
}

block_no_t superblock_data_block_free_stack(const superblock_t* sb)
{
    return sb->data_block_free_stack;
}

void superblock_data_block_free_stack_set(superblock_t* sb, block_no_t new_stack)
{
    sb->data_block_free_stack = new_stack;
}

size_t superblock_block_size(const superblock_t* sb)
{
    return sb->sectors_per_block * BYTES_PER_SECTOR;
}

int superblock_sectors_per_block(const superblock_t *sb)
{
    return sb->sectors_per_block;
}
