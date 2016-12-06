#include "inode.h"

#include "../utils/math.h"
#include "block.h"
#include <assert.h>
#include <string.h>

/*NOTE: 同理，一个简单的实现，没考虑内存布局和字节序 */
size_t inode_bin_size(void)
{
    return sizeof(inode_t);
}

void inode_init(inode_t* inode)
{
    inode->mode = 0;
}



static void inode_no_to_block_no_and_offset(int sectors_per_block, inode_no_t no, block_no_t inode_table_start, block_no_t* p_block_no, size_t* p_offset);

bool inode_load(device_handle_t device,int sectors_per_block, block_no_t inode_table_start, inode_no_t no, inode_t* inode)
{
    block_no_t block_no;
    size_t offset;
    inode_no_to_block_no_and_offset(sectors_per_block, no, inode_table_start, &block_no, &offset);

    char buf[MAX_BYTES_PER_BLOCK];
    bool sucess = block_read(device, sectors_per_block, block_no, buf);
    if (!sucess) {
        return false;
    }

    *inode = *((inode_t*)(buf + offset));

    return true;
}

bool inode_dump(device_handle_t device,int sectors_per_block, block_no_t inode_table_start, inode_no_t no, inode_t* inode)
{
    block_no_t block_no;
    size_t offset;
    inode_no_to_block_no_and_offset(sectors_per_block, no, inode_table_start, &block_no, &offset);


    char buf[MAX_BYTES_PER_BLOCK] = {'\0'};
    *((inode_t*)(buf + offset)) = *inode;

    bool sucess = block_write(device, sectors_per_block, block_no, buf);
    if (!sucess) {
        return false;
    }

    return true;
}

/* 把inode号转为block号和block内偏移 */
static void inode_no_to_block_no_and_offset(int sectors_per_block, inode_no_t no, block_no_t inode_table_start, block_no_t* p_block_no, size_t* p_offset)
{
    assert(no < INODE_MAX_COUNT);

    int inode_num_per_block = sectors_per_block * BYTES_PER_SECTOR / inode_bin_size();
    int inode_blocksize = inode_block_count(sectors_per_block, INODE_MAX_COUNT);
    int block_offset = inode_block_count(sectors_per_block, no) - 1;

    assert(block_offset < inode_blocksize);

    *p_block_no = inode_table_start + block_offset;
    *p_offset = (no - (inode_num_per_block * block_offset)) * inode_bin_size();
}

int inode_block_count(int sectors_per_block, int inode_count)
{
    return count_groups(inode_count, sectors_per_block * BYTES_PER_SECTOR / inode_bin_size());
}
