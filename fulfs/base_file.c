#include "base_file.h"

#include "inode.h"
#include "data_block.h"
#include "block.h"
#include "../utils/math.h"
#include <string.h>

#include <assert.h>

/* 索引等级。1级索引？2级索引？三级索引？ */
static int indirect_level(const base_file_t* base_file,  block_no_t block_relative);
static bool locate(base_file_t* base_file, block_no_t block_relative, block_no_t *p_block);
static bool add_block(base_file_t* base_file);

/* 用来标识文件的某个block */
struct block_info_s
{
    int level;
    int index_in_block0;
    block_no_t block_no_1;
    int index_in_block1;
    block_no_t block_no_2;
    int index_in_block2;
};
static bool block_relative_to_block_info(const base_file_t* base_file, block_no_t block_relative, struct block_info_s* info);

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

fsize_t base_file_tell(const base_file_t* base_file)
{
    return base_file->current.current_block_relative * base_file->dev_inode_ctrl.block_size
        + base_file->current.current_offset;
}

int base_file_read(base_file_t* base_file, int count, char* buf)
{
    /* 保证接下来的count没有超过文件大小 */
    if (base_file_size(base_file) - base_file_tell(base_file) < (fsize_t)count) {
        count = base_file_size(base_file) - base_file_tell(base_file);
    }

    char block_buf[MAX_BYTES_PER_BLOCK];
    int readed_count = 0;
    while (readed_count < count) {
        bool success = block_read(base_file->dev_inode_ctrl.device,
                   base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                   base_file->current.current_block,
                   block_buf);
        if (!success) {
            return BASE_FILE_IO_ERROR;
        }

        int should_read_size = min_int(base_file->dev_inode_ctrl.block_size - base_file->current.current_offset,
                                       count - readed_count);

        memcpy(buf + readed_count, block_buf + base_file->current.current_offset, should_read_size);
        readed_count += should_read_size;

        /* 更新当前的位置 */
        base_file->current.current_offset += should_read_size;
        if (base_file->current.current_offset >= (int)base_file->dev_inode_ctrl.block_size) {
            base_file->current.current_offset = 0;
            base_file->current.current_block_relative++;
            bool success = locate(base_file,
                                  base_file->current.current_block_relative, &(base_file->current.current_block));
            if (!success) {
                return BASE_FILE_IO_ERROR;
            }
        }
    }

    return readed_count;
}

int base_file_write(base_file_t* base_file, int count, const char* buf)
{
    /* 占用为0的文件，要先分配block */
    if (base_file->inode.size == 0) {
    }

    while (true) {
        
    }
    return BASE_FILE_IO_ERROR;
}

bool base_file_close(base_file_t* base_file)
{
    return inode_dump(&(base_file->dev_inode_ctrl), base_file->inode_no, &(base_file->inode));
}

/*********************************/
static int indirect_level(const base_file_t* base_file,  block_no_t block_relative)
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
    struct block_info_s info;
    bool success = block_relative_to_block_info(base_file, block_relative, &info);
    if (!success) {
        return false;
    }

    if (info.level == 0) {
        *p_block = base_file->inode.blocks[block_relative];
        return true;
    }

    /* 最后一跳的间接块 */
    block_no_t last_block;
    int offset;
    if (info.level == 1) {
        last_block = base_file->inode.single_indirect_block;
        offset = info.index_in_block0;
    } else if (info.level == 2) {
        last_block = info.block_no_1;
        offset = info.index_in_block1;
    } else if (info.level == 3) {
        last_block = info.block_no_2;
        offset = info.index_in_block2;
    } else {
        assert(false);
    }


    block_no_t blocks[MAX_NUM_PER_INDIRECT];
    success = indirect_load(base_file->dev_inode_ctrl.device,
                                 base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                                 last_block,
                                 blocks);
    if (!success) {
        return false;
    }
    *p_block = blocks[offset];
    return true;
}

static bool add_block(base_file_t* base_file)
{
    /* TODO */
    return false;
}

static bool block_relative_to_block_info(const base_file_t* base_file, block_no_t block_relative, struct block_info_s* info)
{
    info->level = indirect_level(base_file, block_relative);

    int blocknos_per_block = base_file->dev_inode_ctrl.block_size / sizeof(block_no_t);
    block_no_t level_0_max_block_count = (fsize_t)(LEVEL_0_INDIRECT_COUNT);
    block_no_t level_1_max_block_count = level_0_max_block_count + blocknos_per_block;
    block_no_t level_2_max_block_count = level_1_max_block_count + (blocknos_per_block * blocknos_per_block);

    if (info->level == 0) {
        return true;

    } else if (info->level == 1) {
        info->index_in_block0 =  block_relative - level_0_max_block_count;
        return true;

    } else if (info->level == 2) {
        block_no_t relative = block_relative - level_1_max_block_count;
        block_no_t relative1 = relative / blocknos_per_block;
        int offset1 = relative % blocknos_per_block;

        info->index_in_block0 = relative1;
        info->index_in_block1 = offset1;


        block_no_t blocks[MAX_NUM_PER_INDIRECT];
        bool success = indirect_load(base_file->dev_inode_ctrl.device,
                      base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                      base_file->inode.double_indirect_block,
                      blocks);
        if (!success) {
            return false;
        }

        info->block_no_1 = blocks[info->index_in_block0];
        return true;

    } else if (info->level == 3) {
        block_no_t relative = block_relative - level_2_max_block_count;
        block_no_t relative2 = relative / blocknos_per_block;
        int offset2 = relative % blocknos_per_block;
        block_no_t relative1 = relative2 / blocknos_per_block;
        int offset1 = relative2 % blocknos_per_block;

        info->index_in_block0 = relative1;
        info->index_in_block1 = offset1;
        info->index_in_block2 = offset2;

        block_no_t blocks[MAX_NUM_PER_INDIRECT];
        bool success = indirect_load(base_file->dev_inode_ctrl.device,
                                     base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                                     base_file->inode.triple_indirect_block,
                                     blocks);
        if (!success) {
            return false;
        }

        info->block_no_1 = blocks[info->index_in_block0];
        success = indirect_load(base_file->dev_inode_ctrl.device,
                                base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                                info->block_no_1,
                                blocks);
        if (!success) {
            return false;
        }

        info->block_no_2 = blocks[info->index_in_block1];
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
