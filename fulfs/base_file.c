#include "base_file.h"

#include "inode.h"
#include "data_block.h"
#include "block.h"

#include <assert.h>

/* 索引等级。1级索引？2级索引？三级索引？ */
static int indirect_level(base_file_t* base_file,  block_no_t block_relative);
static bool locate(base_file_t* base_file, block_no_t block_relative, block_no_t *p_block);

static bool indirect_load(device_handle_t device, int sectors_per_block, block_no_t block, block_no_t blocks[]);
static bool indirect_dump(device_handle_t device, int sectors_per_block, block_no_t block, block_no_t blocks[]);
#define MAX_NUM_PER_INDIRECT (MAX_BYTES_PER_BLOCK / sizeof(block_no_t))
static int num_per_indirect(int block_size);

bool base_file_open(base_file_t* base_file, dev_inode_ctrl_t* dev_inode_ctrl, inode_no_t inode_no)
{
    base_file->dev_inode_ctrl = *dev_inode_ctrl;
    bool success = inode_load(dev_inode_ctrl, inode_no, &(base_file->inode));
    if (!success) {
        return false;
    }

    base_file->current.current_block = base_file->inode.blocks[0];
    base_file->current.current_offset = 0;
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

    block_no_t block_relative = offset / base_file->dev_inode_ctrl.block_size;
    base_file->current.current_block_relative = block_relative;
    base_file->current.current_offset = offset % base_file->dev_inode_ctrl.block_size;

    bool success = locate(base_file, block_relative, &(base_file->current.current_block));
    return success;
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
static int indirect_level(base_file_t* base_file,  block_no_t block_relative)
{
    block_no_t level_0_max_block_count = (fsize_t)(LEVEL_0_INDIRECT_COUNT);
    if (block_relative <= level_0_max_block_count) {
        return 0;
    }

    int blocknos_per_block = base_file->dev_inode_ctrl.block_size / sizeof(block_no_t);

    block_no_t level_1_max_block_count = level_0_max_block_count + blocknos_per_block;
    if (block_relative <= level_1_max_block_count) {
        return 1;
    }

    block_no_t level_2_max_block_count = level_1_max_block_count + (blocknos_per_block * blocknos_per_block);
    if (block_relative <= level_2_max_block_count) {
        return 2;
    }

    block_no_t level_3_max_block_count = level_2_max_block_count + (blocknos_per_block * blocknos_per_block * blocknos_per_block);
    if (block_relative <= level_3_max_block_count) {
        return 3;
    }

    assert(false);
    return 0;
}

static bool locate(base_file_t* base_file, block_no_t block_relative, block_no_t *p_block)
{

    int blocknos_per_block = base_file->dev_inode_ctrl.block_size / sizeof(block_no_t);
    block_no_t level_0_max_block_count = (fsize_t)(LEVEL_0_INDIRECT_COUNT);
    block_no_t level_1_max_block_count = level_0_max_block_count + blocknos_per_block;
    block_no_t level_2_max_block_count = level_1_max_block_count + (blocknos_per_block * blocknos_per_block);

    int level = indirect_level(base_file, block_relative);
    if (level == 0) {
        *p_block = base_file->inode.blocks[block_relative];
        return true;

    } else if (level == 1) {
        block_no_t blocks[MAX_NUM_PER_INDIRECT];
        bool success = indirect_load(base_file->dev_inode_ctrl.device,
                   base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                   base_file->inode.single_indirect_block,
                   blocks);
        if (!success) {
            return false;
        }

        *p_block = blocks[block_relative - level_0_max_block_count];
        return true;

    } else if (level == 2) {
        block_no_t relative = block_relative - level_1_max_block_count;
        block_no_t relative1 = relative / blocknos_per_block;
        int offset1 = relative % blocknos_per_block;


        block_no_t blocks[MAX_NUM_PER_INDIRECT];
        bool success = indirect_load(base_file->dev_inode_ctrl.device,
                      base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                      base_file->inode.double_indirect_block,
                      blocks);
        if (!success) {
            return false;
        }

        block_no_t block = blocks[relative1];
        success = indirect_load(base_file->dev_inode_ctrl.device,
                      base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                      block,
                      blocks);
        if (!success) {
            return false;
        }

        *p_block = blocks[offset1];
        return true;

    } else if (level == 3) {
        block_no_t relative = block_relative - level_2_max_block_count;
        block_no_t relative2 = relative / blocknos_per_block;
        int offset2 = relative % blocknos_per_block;
        block_no_t relative1 = relative2 / blocknos_per_block;
        int offset1 = relative2 % blocknos_per_block;


        block_no_t blocks[MAX_NUM_PER_INDIRECT];
        bool success = indirect_load(base_file->dev_inode_ctrl.device,
                                     base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                                     base_file->inode.triple_indirect_block,
                                     blocks);
        if (!success) {
            return false;
        }

        block_no_t block1 = blocks[relative1];
        success = indirect_load(base_file->dev_inode_ctrl.device,
                                base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                                block1,
                                blocks);
        if (!success) {
            return false;
        }

        block_no_t block2 = blocks[offset1];
        success = indirect_load(base_file->dev_inode_ctrl.device,
                                base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                                block2,
                                blocks);
        if (!success) {
            return false;
        }

        *p_block = blocks[offset2];
        return true;
    } else {
        assert(false);
    }

    assert(false);
}


/************************************************/
/* NOTE: 简单实现，未考虑字节序 */
static bool indirect_load(device_handle_t device, int sectors_per_block, block_no_t block, block_no_t blocks[])
{
    char buf[MAX_BYTES_PER_BLOCK];
    bool success = block_read(device, sectors_per_block, block, buf);
    if (!success) {
        return false;
    }

    int num = num_per_indirect(sectors_per_block * BYTES_PER_SECTOR);
    for (int i = 0; i < num; i++) {
        blocks[i] = *(buf + sizeof(block_no_t));
    }

    return true;
}

static bool indirect_dump(device_handle_t device, int sectors_per_block, block_no_t block, block_no_t blocks[])
{
    char buf[MAX_BYTES_PER_BLOCK];

    int num = num_per_indirect(sectors_per_block * BYTES_PER_SECTOR);
    for (int i = 0; i < num; i++) {
        *(buf + sizeof(block_no_t)) = blocks[i];
    }
    return block_write(device, sectors_per_block, block, buf);
}

static int num_per_indirect(int block_size)
{
    return block_size / sizeof(block_no_t);
}
