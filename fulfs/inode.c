#include "inode.h"

#include "../utils/math.h"
#include "../utils/log.h"
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


void dev_inode_ctrl_init(dev_inode_ctrl_t* dev_inode_ctrl, device_handle_t device, size_t block_size, block_no_t start, block_no_t size)
{
    dev_inode_ctrl->device = device;
    dev_inode_ctrl->block_size = block_size;
    dev_inode_ctrl->start = start;
    dev_inode_ctrl->size = size;
}

void dev_inode_ctrl_init_from_superblock(dev_inode_ctrl_t* dev_inode_ctrl, device_handle_t device, const superblock_t* sb)
{
    dev_inode_ctrl_init(dev_inode_ctrl, device, superblock_block_size(sb), superblock_inode_table_start(sb), superblock_inode_table_blocksize(sb));
}

static void inode_no_to_block_no_and_offset(dev_inode_ctrl_t *dev_inode_ctrl, inode_no_t no, block_no_t* p_block_no, size_t* p_offset);
bool inode_load(dev_inode_ctrl_t* dev_inode_ctrl, inode_no_t no, inode_t* inode)
{
    block_no_t block_no;
    size_t offset;
    inode_no_to_block_no_and_offset(dev_inode_ctrl, no, &block_no, &offset);

    char buf[MAX_BYTES_PER_BLOCK];
    bool sucess = block_read(dev_inode_ctrl->device, dev_inode_ctrl->block_size / BYTES_PER_SECTOR, block_no, buf);
    if (!sucess) {
        return false;
    }

    *inode = *((inode_t*)(buf + offset));

    return true;
}

bool inode_dump(dev_inode_ctrl_t * dev_inode_ctrl, inode_no_t no, inode_t* inode)
{
    block_no_t block_no;
    size_t offset;
    inode_no_to_block_no_and_offset(dev_inode_ctrl, no, &block_no, &offset);

    char buf[MAX_BYTES_PER_BLOCK] = {'\0'};
    bool sucess = block_read(dev_inode_ctrl->device, dev_inode_ctrl->block_size / BYTES_PER_SECTOR, block_no, buf);
    if (!sucess) {
        return false;
    }

    *((inode_t*)(buf + offset)) = *inode;

    sucess = block_write(dev_inode_ctrl->device, dev_inode_ctrl->block_size / BYTES_PER_SECTOR, block_no, buf);
    if (!sucess) {
        log_debug("inode写入错误，block写入失败: %d", (int)block_no);
        return false;
    }

    return true;
}

/* 把inode号转为block号和block内偏移 */
static void inode_no_to_block_no_and_offset(dev_inode_ctrl_t *dev_inode_ctrl, inode_no_t no, block_no_t* p_block_no, size_t* p_offset)
{
    assert(no < INODE_MAX_COUNT);

    int inode_num_per_block = dev_inode_ctrl->block_size / inode_bin_size();
    int inode_blocksize = inode_block_count(dev_inode_ctrl->block_size, INODE_MAX_COUNT);
    int block_relative = no / inode_num_per_block;

    assert(block_relative < inode_blocksize);

    *p_block_no = dev_inode_ctrl->start + block_relative;
    *p_offset = (no - (inode_num_per_block * block_relative)) * inode_bin_size();
}

int inode_block_count(size_t block_size, int inode_count)
{
    return count_groups(inode_count, block_size / inode_bin_size());
}

bool inode_alloc(dev_inode_ctrl_t* dev_inode_ctrl, inode_no_t* p_inode_no)
{
    inode_t inode;
    for (inode_no_t i = 0; i < INODE_MAX_COUNT; i++) {
        bool success = inode_load(dev_inode_ctrl, i, &inode);
        if (!success) {
            return false;
        }
        if (inode.mode == 0) {
            *p_inode_no = i;
            return true;
        }
    }
    return false;
}

void inode_free(dev_inode_ctrl_t* dev_inode_ctrl, inode_no_t no)
{
    inode_t inode;
    bool success = inode_load(dev_inode_ctrl, no, &inode);
    if (success) {
        inode.mode = 0;
    }
    inode_dump(dev_inode_ctrl, no, &inode);
}
