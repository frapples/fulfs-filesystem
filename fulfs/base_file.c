#include "base_file.h"

#include "inode.h"
#include "data_block.h"

#include <assert.h>

/* 索引等级。1级索引？2级索引？三级索引？ */
static int indirect_level(base_file_t* base_file);

bool base_file_open(base_file_t* base_file, dev_inode_ctrl_t* dev_inode_ctrl, inode_no_t inode_no)
{
    base_file->dev_inode_ctrl = *dev_inode_ctrl;
    bool success = inode_load(dev_inode_ctrl, inode_no, &(base_file->inode));
    if (!success) {
        return false;
    }

    base_file->current_block = base_file->inode.blocks[0];
    base_file->current_offset = 0;
    return true;
}

int base_file_mode(const base_file_t* base_file)
{
    return base_file->inode.mode;
}

fsize_t base_file_size(const base_file_t* base_file)
{
    return base_file->inode.size;
}

time_t base_file_accessed_time(const base_file_t* base_file)
{
    return base_file->inode.accessed_time;
}

time_t base_file_modified_time(const base_file_t* base_file)
{
    return base_file->inode.modified_time;
}

time_t base_file_created_time(const base_file_t* base_file)
{
    return base_file->inode.created_time;
}

bool base_file_seek(base_file_t* base_file, fsize_t offset)
{
    assert(offset <= base_file->inode.size);
    /* TODO */
    return true;
}

bool base_file_read(base_file_t* base_file, int count, char* buf)
{
    /* TODO */
    return true;
}

bool base_file_write(base_file_t* base_file, int count, const char* buf)
{
    /* TODO */
    return true;
}

bool base_file_close(base_file_t* base_file)
{
    return inode_dump(&(base_file->dev_inode_ctrl), base_file->inode_no, &(base_file->inode));
}

/*********************************/
static int indirect_level(base_file_t* base_file)
{
    fsize_t file_size = base_file->inode.size;
    int block_size = base_file->dev_inode_ctrl.block_size;

    fsize_t level_0_max_size = (fsize_t)(LEVEL_0_INDIRECT_COUNT * block_size);
    if (file_size <= level_0_max_size) {
        return 0;
    }

    int blocknos_per_block = base_file->dev_inode_ctrl.block_size / sizeof(block_no_t);

    fsize_t level_1_max_size = level_0_max_size + blocknos_per_block * block_size;
    if (file_size <= level_1_max_size) {
        return 1;
    }

    fsize_t level_2_max_size = level_1_max_size + (blocknos_per_block * blocknos_per_block * block_size);
    if (file_size <= level_2_max_size) {
        return 2;
    }

    fsize_t level_3_max_size = level_2_max_size + (blocknos_per_block * blocknos_per_block * blocknos_per_block * block_size);
    if (file_size <= level_3_max_size) {
        return 3;
    }

    assert(false);
    return 0;
}
