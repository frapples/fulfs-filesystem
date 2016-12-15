#include "base_file.h"

#include "inode.h"
#include "data_block.h"
#include "block.h"
#include "../utils/math.h"
#include <string.h>

#include <assert.h>

/* TODO 关于时间的部分没管它 */

static bool base_file_del(device_handle_t device, superblock_t* sb, inode_no_t inode_no);

/* 索引等级。1级索引？2级索引？三级索引？ */
static int indirect_level(const base_file_t* base_file,  block_no_t block_relative);
/* 根据相对block号定位具体储存数据的block号 */
static bool locate(base_file_t* base_file, block_no_t block_relative, block_no_t *p_block);
/* 给文件添加一个储存block */
static bool add_block(base_file_t* base_file, block_no_t* p_newblock);

/***********************************************/
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
static bool add_a_item_for_indirect(base_file_t* base_file, block_no_t no, int size, block_no_t *p_block); /* 给一个间接块添加一项 */
static bool alloc_block(base_file_t* base_file, block_no_t* p_block); /* 包装下data block那一挂参数的函数 =_= */
static bool free_block(base_file_t* base_file, block_no_t block);

/************************************************/
static bool indirect_load(device_handle_t device, int sectors_per_block, block_no_t block, block_no_t blocks[]);
static bool indirect_dump(device_handle_t device, int sectors_per_block, block_no_t block, block_no_t blocks[]);
#define MAX_NUM_PER_INDIRECT (MAX_BYTES_PER_BLOCK / sizeof(block_no_t))
static int num_per_indirect(int block_size);

bool base_file_open(base_file_t* base_file, device_handle_t device, superblock_t* sb, inode_no_t inode_no)
{
    dev_inode_ctrl_init_from_superblock(&(base_file->dev_inode_ctrl), device, sb);
    base_file->sb = *sb;
    bool success = inode_load(&(base_file->dev_inode_ctrl), inode_no, &(base_file->inode));
    if (!success) {
        return false;
    }

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

timestamp_t base_file_accessed_time(const base_file_t* base_file)
{
    return base_file->inode.accessed_time;
}

timestamp_t base_file_modified_time(const base_file_t* base_file)
{
    return base_file->inode.modified_time;
}

timestamp_t base_file_created_time(const base_file_t* base_file)
{
    return base_file->inode.created_time;
}

bool base_file_seek(base_file_t* base_file, fsize_t offset)
{
    assert(offset <= base_file->inode.size);

    block_no_t block_relative = offset / base_file->dev_inode_ctrl.block_size;
    base_file->current.current_block_relative = block_relative;
    base_file->current.current_offset = offset % base_file->dev_inode_ctrl.block_size;
    return true;
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
        block_no_t current_block;
        bool success = locate(base_file, base_file->current.current_block_relative, &current_block);
        if (!success) {
            return BASE_FILE_IO_ERROR;
        }

        success = block_read(base_file->dev_inode_ctrl.device,
                             base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                             current_block,
                             block_buf);
        if (!success) {
            return BASE_FILE_IO_ERROR;
        }

        int should_read_size = min_int(base_file->dev_inode_ctrl.block_size - base_file->current.current_offset,
                                       count - readed_count);

        memcpy(buf + readed_count, block_buf + base_file->current.current_offset, should_read_size);
        readed_count += should_read_size;

        base_file_seek(base_file, base_file_tell(base_file) + should_read_size);
    }

    return readed_count;
}

int base_file_write(base_file_t* base_file, int count, const char* buf)
{
    char block_buf[MAX_BYTES_PER_BLOCK];
    int writed_count = 0;
    while (writed_count < count) {

        block_no_t current_block;
        if (base_file_tell(base_file) == base_file_size(base_file)) {
            /* FIXME: 这里有同样尴尬的问题，就是如果后续失败了这里分配的block怎么办 */
            bool success = add_block(base_file, &current_block);
            if (!success) {
                return false;
            }
        } else {
            bool success = locate(base_file, base_file->current.current_block_relative, &current_block);
            if (!success) {
                return BASE_FILE_IO_ERROR;
            }
        }
        bool success = block_read(base_file->dev_inode_ctrl.device,
                                  base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                                  current_block,
                                  block_buf);
        if (!success) {
            return BASE_FILE_IO_ERROR;
        }

        int should_write_size = min_int(base_file->dev_inode_ctrl.block_size - base_file->current.current_offset,
                                       count - writed_count);

        memcpy(block_buf + base_file->current.current_offset, buf + writed_count, should_write_size);

        success = block_write(base_file->dev_inode_ctrl.device,
                                  base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                                  current_block,
                                  block_buf);
        if (!success) {
            return BASE_FILE_IO_ERROR;
        }

        writed_count += should_write_size;
        base_file->inode.size += should_write_size;
        base_file_seek(base_file, base_file_tell(base_file) + should_write_size);
    }
    return BASE_FILE_IO_ERROR;
}

bool base_file_close(base_file_t* base_file)
{
    return inode_dump(&(base_file->dev_inode_ctrl), base_file->inode_no, &(base_file->inode));
}

bool base_file_create(device_handle_t device, superblock_t* sb, int mode, inode_no_t* p_inode_no)
{
    dev_inode_ctrl_t dev_inode_ctrl;
    dev_inode_ctrl_init_from_superblock(&dev_inode_ctrl, device, sb);

    bool success = inode_alloc(&dev_inode_ctrl, p_inode_no);
    if (!success) {
        return false;
    }

    inode_t inode;
    success = inode_load(&dev_inode_ctrl, *p_inode_no, &inode);
    if (!success) {
        return false;
    }

    inode.mode = mode;
    inode.size = 0;
    inode.link_count = 1;

    success = inode_dump(&dev_inode_ctrl, *p_inode_no, &inode);
    if (!success) {
        return false;
    }

    return true;
}

bool base_file_ref(device_handle_t device, superblock_t* sb, inode_no_t inode_no)
{
    dev_inode_ctrl_t dev_inode_ctrl;
    dev_inode_ctrl_init_from_superblock(&dev_inode_ctrl, device, sb);

    inode_t inode;
    bool success = inode_load(&dev_inode_ctrl, inode_no, &inode);
    if (!success) {
        return false;
    }

    inode.link_count++;

    success = inode_dump(&dev_inode_ctrl, inode_no, &inode);
    if (!success) {
        return false;
    }

    return true;
}

bool base_file_unref(device_handle_t device, superblock_t* sb, inode_no_t inode_no)
{
    dev_inode_ctrl_t dev_inode_ctrl;
    dev_inode_ctrl_init_from_superblock(&dev_inode_ctrl, device, sb);

    inode_t inode;
    bool success = inode_load(&dev_inode_ctrl, inode_no, &inode);
    if (!success) {
        return false;
    }

    inode.link_count--;
    if (inode.link_count > 0) {
        success = inode_dump(&dev_inode_ctrl, inode_no, &inode);
        if (!success) {
            return false;
        }
    } else {
        return base_file_del(device, sb, inode_no);
    }

    return true;
}

bool base_file_truncate(base_file_t* base_file, fsize_t size)
{
    /* 这个函数要保证出错时不破坏完整性 */
    if (base_file->inode.size > size) {

        assert(base_file->inode.size > 0);

        int block_size = base_file->dev_inode_ctrl.block_size;

        block_no_t block_num = count_groups(base_file->inode.size, block_size);
        block_no_t should_block_num = count_groups(size, block_size);

        for (block_no_t i = 0; i < block_num - should_block_num; i++) {

            block_no_t block_relative = (block_num - 1) - i;

            block_no_t block;
            bool success = locate(base_file, block_relative, &block);
            if (!success) {
                return false;
            }
            success = free_block(base_file, block);
            if (!success) {
                return false;
            }
            base_file->inode.size -= block_size;
        }

        base_file->inode.size = size;
        return true;

    } else {
        return true;
    }
}

/*********************************/
static bool base_file_del(device_handle_t device, superblock_t* sb, inode_no_t inode_no)
{
    base_file_t base_file;
    bool success = base_file_open(&base_file, device, sb, inode_no);
    if (!success) {
        return false;
    }
    /* 释放block */
    success = base_file_truncate(&base_file, 0);
    if (!success) {
        return false;
    }
    base_file_close(&base_file);

    /* 释放inode */
    inode_free(&(base_file.dev_inode_ctrl), inode_no);
    return true;
}

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

static bool add_block(base_file_t* base_file, block_no_t* p_newblock)
{
    /* NOTE: 目前的实现很啰嗦 =_= 而且如果分配了几个块后出现错误，会导致类似内存泄漏的块泄漏问题 以后再改吧 */
    /* FIXME:可能造成块泄漏 */
    bool success;
    if (base_file_size(base_file) == 0) {
        /* 特殊处理为文件大小0的情况 */
        success = alloc_block(base_file, &(base_file->inode.blocks[0]));
        if (!success) {
            return false;
        }
        *p_newblock = base_file->inode.blocks[0];
        return true;
    }

    block_no_t last_block_relative = (base_file_size(base_file) - 1) / base_file->dev_inode_ctrl.block_size;
    int blocknos_per_block = num_per_indirect(base_file->dev_inode_ctrl.block_size);

    struct block_info_s info;
    success = block_relative_to_block_info(base_file, last_block_relative, &info);
    if (!success) {
        return false;
    }


    if (indirect_level(base_file, last_block_relative) == indirect_level(base_file, last_block_relative + 1)) {
        /* 加一个块不会导致启用更高的索引 */
        if (info.level == 0) {
            success = alloc_block(base_file, &(base_file->inode.blocks[last_block_relative + 1]));
            if (!success) {
                return false;
            }
            *p_newblock = base_file->inode.blocks[last_block_relative + 1];
            return true;
        } else if (info.level == 1) {

            return add_a_item_for_indirect(base_file, base_file->inode.single_indirect_block, info.index_in_block0, p_newblock);

        } else if (info.level == 2) {

            if (info.index_in_block1 < blocknos_per_block - 1) {
                /* 没满 */
                return add_a_item_for_indirect(base_file, info.block_no_1, info.index_in_block1, p_newblock);
            } else {
                block_no_t block1;
                success = add_a_item_for_indirect(base_file, base_file->inode.double_indirect_block, info.index_in_block0, &block1);
                if (!success) {
                    return false;
                }
                return add_a_item_for_indirect(base_file, block1, 0, p_newblock);
            }

        } else if (info.level == 3) {

            if (info.index_in_block2 < blocknos_per_block - 1) {
                /* 没满 */
                return add_a_item_for_indirect(base_file, info.block_no_2, info.index_in_block2, p_newblock);
            }

            /* 需要额外分配一片新的第二跳 */
            block_no_t block2;
            if (info.index_in_block1 < blocknos_per_block - 1) {
                success = add_a_item_for_indirect(base_file, info.block_no_1, info.index_in_block1, &block2);
                if (!success) {
                    return false;
                }
            } else {
                block_no_t block1;
                success = add_a_item_for_indirect(base_file, base_file->inode.triple_indirect_block, info.index_in_block0, &block1);
                if (!success) {
                    return false;
                }
                success = add_a_item_for_indirect(base_file, block1, 0, &block2);
                if (!success) {
                    return false;
                }
            }

            return add_a_item_for_indirect(base_file, block2, 0, p_newblock);

        } else {

            assert(false);
        }


    } else {


        if (info.level == 0) {
            /* 启用1级索引 */

            success = alloc_block(base_file,  &(base_file->inode.single_indirect_block));
            if (!success) {
                return false;
            }
            return add_a_item_for_indirect(base_file, base_file->inode.single_indirect_block, 0, p_newblock);

        } else if (info.level == 1) {

            success = alloc_block(base_file,  &(base_file->inode.double_indirect_block));
            if (!success) {
                return false;
            }

            block_no_t block1;
            success = add_a_item_for_indirect(base_file, base_file->inode.double_indirect_block, 0, &block1);
            if (!success) {
                return false;
            }
            return add_a_item_for_indirect(base_file, block1, 0, p_newblock);

        } else if (info.level == 2) {

            success = alloc_block(base_file,  &(base_file->inode.triple_indirect_block));
            if (!success) {
                return false;
            }

            block_no_t block1;
            success = add_a_item_for_indirect(base_file, base_file->inode.double_indirect_block, 0, &block1);
            if (!success) {
                return false;
            }

            block_no_t block2;
            success = add_a_item_for_indirect(base_file, block1, 0, &block2);
            if (!success) {
                return false;
            }

            return add_a_item_for_indirect(base_file, block2, 0, p_newblock);

        } else {
            assert(false);
        }
    }
    assert(false);
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

static bool add_a_item_for_indirect(base_file_t* base_file, block_no_t no, int size, block_no_t *p_block)
{
    assert(size < num_per_indirect(base_file->dev_inode_ctrl.block_size) - 1);

    block_no_t blocks[MAX_NUM_PER_INDIRECT];
    bool success = indirect_load(base_file->dev_inode_ctrl.device, base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR, no, blocks);
    if (!success) {
        return false;
    }

    success = alloc_block(base_file, &(blocks[size]));
    if (!success) {
        return false;
    }

    success = indirect_dump(base_file->dev_inode_ctrl.device, base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR, no, blocks);
    if (!success) {
        free_block(base_file, blocks[size]);
        return false;
    }

    *p_block = blocks[size];
    return true;
}

static bool alloc_block(base_file_t* base_file, block_no_t* p_block)
{
    /* 保证出错时不修改p_block */
    block_no_t block;
    bool success = data_block_alloc(base_file->dev_inode_ctrl.device,
                                    base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                                    superblock_data_block_free_stack(&(base_file->sb)),
                                    &block);
    if (success) {
        *p_block = block;
    }
    return success;
}


static bool free_block(base_file_t* base_file, block_no_t block)
{
    return data_block_free(base_file->dev_inode_ctrl.device,
                                    base_file->dev_inode_ctrl.block_size / BYTES_PER_SECTOR,
                                    superblock_data_block_free_stack(&(base_file->sb)),
                                    block);
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
