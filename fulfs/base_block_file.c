#include "base_block_file.h"

#include "../utils/math.h"

#include <string.h>
#include <assert.h>


/* 把间接索引方式看成一颗树，将第level层进行编号得no，得到该树中该节点对应的block */
static bool locate(device_handle_t device, int sectors_per_block, block_no_t first, int level, block_no_t no, block_no_t* p_block);
/* 给这颗树加叶子
   其中, device, sector_per_block不解释，data_blocks_stack参数是分配新块需要
   p_first是树的根节点
   level 是索引等级
   size 是当前的树有多少叶子
 */
static bool add(device_handle_t device, int sectors_per_block, block_no_t data_blocks_stack,
                block_no_t* p_first, int level, block_no_t size, block_no_t* p_block, block_no_t* p_used_block_count);
/* 给这颗树剪叶子 */
static bool pop(device_handle_t device, int sectors_per_block, block_no_t data_blocks_stack,
                block_no_t* p_first, int level, block_no_t size, block_no_t* p_used_block_count);


#define MAX_NUM_PER_INDIRECT (MAX_BYTES_PER_BLOCK / sizeof(block_no_t))
static bool indirect_load(device_handle_t device, int sectors_per_block, block_no_t block, block_no_t blocks[]);
static bool indirect_dump(device_handle_t device, int sectors_per_block, block_no_t block, block_no_t blocks[]);
static int num_per_indirect(int block_size);


bool base_block_file_locate(device_handle_t device,  superblock_t* sb, inode_t* inode, block_no_t block_relative, block_no_t *p_block)
{
    assert(inode->size > 0);

    int blocknos_per_block = num_per_indirect(superblock_block_size(sb));
    block_no_t level_0_max_block_count = (fsize_t)(LEVEL_0_INDIRECT_COUNT);
    block_no_t level_1_max_block_count = level_0_max_block_count + blocknos_per_block;
    block_no_t level_2_max_block_count = level_1_max_block_count + (blocknos_per_block * blocknos_per_block);
    block_no_t level_3_max_block_count = level_2_max_block_count + (blocknos_per_block * blocknos_per_block * blocknos_per_block);

    if (block_relative < level_0_max_block_count) {
        *p_block = inode->blocks[block_relative];
        return true;
    } else if (block_relative < level_1_max_block_count) {
        return locate(device, superblock_sectors_per_block(sb), inode->single_indirect_block, 1, block_relative - level_0_max_block_count, p_block);
    } else if (block_relative < level_2_max_block_count) {
        return locate(device, superblock_sectors_per_block(sb), inode->double_indirect_block, 2, block_relative - level_1_max_block_count, p_block);
    } else if (block_relative < level_3_max_block_count) {
        return locate(device, superblock_sectors_per_block(sb), inode->triple_indirect_block, 3, block_relative - level_2_max_block_count, p_block);
    } else {
        assert(false);
    }

    return false;
}

bool base_block_file_push_block(device_handle_t device, superblock_t* sb, inode_t* inode, block_no_t* p_block)
{
    /* 目前的block个数 */
    block_no_t block_count = count_groups(inode->size, superblock_block_size(sb));

    int blocknos_per_block = num_per_indirect(superblock_block_size(sb));
    block_no_t level_0_max_block_count = (fsize_t)(LEVEL_0_INDIRECT_COUNT);
    block_no_t level_1_max_block_count = level_0_max_block_count + blocknos_per_block;
    block_no_t level_2_max_block_count = level_1_max_block_count + (blocknos_per_block * blocknos_per_block);
    block_no_t level_3_max_block_count = level_2_max_block_count + (blocknos_per_block * blocknos_per_block * blocknos_per_block);

    int sectors_per_block = superblock_sectors_per_block(sb);
    block_no_t data_block_stack = superblock_data_block_free_stack(sb);
    if (block_count < level_0_max_block_count) {
        bool success = data_block_alloc(device, sectors_per_block, data_block_stack, p_block, &sb->used_data_block_count);
        inode->blocks[block_count] = *p_block;
        return success;
    } else if (block_count < level_1_max_block_count) {
        return add(device, sectors_per_block, data_block_stack,
                   &inode->single_indirect_block, 1, block_count - level_0_max_block_count, p_block, &sb->used_data_block_count);
    } else if (block_count < level_2_max_block_count) {
        return add(device, sectors_per_block, data_block_stack,
                   &inode->double_indirect_block, 2, block_count - level_1_max_block_count, p_block, &sb->used_data_block_count);
    } else if (block_count < level_3_max_block_count) {
        return add(device, sectors_per_block, data_block_stack,
                   &inode->triple_indirect_block, 3, block_count - level_2_max_block_count, p_block, &sb->used_data_block_count);
    } else {
        assert(false);
    }

    return false;
}

bool base_block_file_pop_block(device_handle_t device, superblock_t* sb, inode_t* inode)
{
    assert(inode->size > 0);

    block_no_t block_count = (inode->size - 1) / superblock_block_size(sb) + 1;

    int blocknos_per_block = num_per_indirect(superblock_block_size(sb));
    block_no_t level_0_max_block_count = (fsize_t)(LEVEL_0_INDIRECT_COUNT);
    block_no_t level_1_max_block_count = level_0_max_block_count + blocknos_per_block;
    block_no_t level_2_max_block_count = level_1_max_block_count + (blocknos_per_block * blocknos_per_block);
    block_no_t level_3_max_block_count = level_2_max_block_count + (blocknos_per_block * blocknos_per_block * blocknos_per_block);

    int sectors_per_block = superblock_sectors_per_block(sb);
    block_no_t data_block_stack = superblock_data_block_free_stack(sb);
    if (block_count <= level_0_max_block_count) {
        block_no_t new_block;
        data_block_free(device, sectors_per_block, data_block_stack, inode->blocks[block_count - 1], &sb->used_data_block_count);
        return data_block_alloc(device, sectors_per_block, data_block_stack, &new_block, &sb->used_data_block_count);
    } else if (block_count <= level_1_max_block_count) {
        return pop(device, sectors_per_block, data_block_stack,
                   &inode->single_indirect_block, 1, block_count - level_0_max_block_count, &sb->used_data_block_count);
    } else if (block_count <= level_2_max_block_count) {
        return pop(device, sectors_per_block, data_block_stack,
                   &inode->double_indirect_block, 2, block_count - level_1_max_block_count, &sb->used_data_block_count);
    } else if (block_count <= level_3_max_block_count) {
        return pop(device, sectors_per_block, data_block_stack,
                   &inode->triple_indirect_block, 3, block_count - level_2_max_block_count, &sb->used_data_block_count);
    } else {
        assert(false);
    }
    return false;
}


static bool locate(device_handle_t device, int sectors_per_block, block_no_t first, int level, block_no_t no, block_no_t* p_block)
{
    if (level == 0) {

        *p_block = first;
        return true;

    } else {

        block_no_t block;
        bool success = locate(device, sectors_per_block,
                              first, level - 1, no / num_per_indirect(sectors_per_block * BYTES_PER_SECTOR), &block);
        if (!success) {
            return false;
        }

        block_no_t blocks[MAX_NUM_PER_INDIRECT];
        success = indirect_load(device, sectors_per_block, block, blocks);
        if (!success) {
            return false;
        }

        *p_block = blocks[no % num_per_indirect(sectors_per_block * BYTES_PER_SECTOR)];
        return true;

    }
}


static bool add(device_handle_t device, int sectors_per_block, block_no_t data_blocks_stack, block_no_t* p_first, int level, block_no_t size, block_no_t* p_block, block_no_t* p_used_block_count)
{
    if (level == 0) {
        bool success = data_block_alloc(device, sectors_per_block, data_blocks_stack, p_block, p_used_block_count);
        if (!success) {
            return false;
        }
        *p_first = *p_block;
        return true;

    } else {

        int blocknos_per_block = num_per_indirect(sectors_per_block * BYTES_PER_SECTOR);
        /* 新加块在上级的offset */
        int offset = size % blocknos_per_block;
        block_no_t block;
        if (offset == 0) {
            /* 上一级没有空余了 */
            bool success = add(device, sectors_per_block, data_blocks_stack,
                               p_first, level - 1, count_groups(size, blocknos_per_block), &block, p_used_block_count);
            if (!success) {
                return false;
            }
        } else {

            assert(size > 0);

            bool success = locate(device, sectors_per_block, *p_first, level - 1, (size - 1) / blocknos_per_block, &block);
            if (!success) {
                return false;
            }
        }

        block_no_t blocks[MAX_NUM_PER_INDIRECT];
        bool success = indirect_load(device, sectors_per_block, block, blocks);
        if (!success) {
            return false;
        }

        success = data_block_alloc(device, sectors_per_block, data_blocks_stack, &(blocks[offset]), p_used_block_count);
        if (!success) {
            return false;
        }

        success = indirect_dump(device, sectors_per_block, block, blocks);
        if (!success) {
            return false;
        }
        *p_block = blocks[offset];
        return true;
    }
}


static bool pop(device_handle_t device, int sectors_per_block, block_no_t data_blocks_stack, block_no_t* p_first, int level, block_no_t size, block_no_t* p_used_block_count)
{
    assert(size > 0);

    if (level == 0) {
        return data_block_free(device, sectors_per_block, data_blocks_stack, *p_first, p_used_block_count);
    } else {

        int blocknos_per_block = num_per_indirect(sectors_per_block * BYTES_PER_SECTOR);
        int offset = (size - 1) / blocknos_per_block;


        block_no_t block;
        bool success = locate(device, sectors_per_block, *p_first, level - 1, offset, &block);
        if (!success) {
            return false;
        }

        if (offset == 0) {
            success = pop(device, sectors_per_block, data_blocks_stack, p_first, level - 1, (size - 1) / blocknos_per_block + 1, p_used_block_count);
            if (!success) {
                return false;
            }
        }

        /* FIXME:这里要是释放失败了，其实是很尴尬的事情，破坏了完整性 */
        success = data_block_free(device, sectors_per_block, data_blocks_stack, *p_first, p_used_block_count);
        if (!success) {
            return false;
        }
        return true;
    }
}


bool base_block_file_block_count(device_handle_t device, superblock_t* sb, inode_t* inode, long* p_count)
{
    /* 目前的block个数 */
    block_no_t block_count = count_groups(inode->size, superblock_block_size(sb));

    int blocknos_per_block = num_per_indirect(superblock_block_size(sb));
    block_no_t level_0_max_block_count = (fsize_t)(LEVEL_0_INDIRECT_COUNT);
    block_no_t level_1_max_block_count = level_0_max_block_count + blocknos_per_block;
    block_no_t level_2_max_block_count = level_1_max_block_count + (blocknos_per_block * blocknos_per_block);
    block_no_t level_3_max_block_count = level_2_max_block_count + (blocknos_per_block * blocknos_per_block * blocknos_per_block);

    *p_count = block_count;
    if (block_count < level_0_max_block_count) {
        *p_count += 0;
    } else if (block_count < level_1_max_block_count) {
        *p_count += 1;
    } else if (block_count < level_2_max_block_count) {
        *p_count += 1 + count_groups(block_count - level_1_max_block_count, blocknos_per_block);
    } else if (block_count < level_3_max_block_count) {
        int indirect_3_count = count_groups(block_count - level_2_max_block_count, blocknos_per_block);
        *p_count += 1 + count_groups(indirect_3_count, blocknos_per_block) + indirect_3_count;
    } else {
        assert(false);
    }
    return true;
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

    memcpy(blocks, buf, sectors_per_block * BYTES_PER_SECTOR);

    return true;
}

static bool indirect_dump(device_handle_t device, int sectors_per_block, block_no_t block, block_no_t blocks[])
{
    char buf[MAX_BYTES_PER_BLOCK];

    memcpy(buf, blocks, sectors_per_block * BYTES_PER_SECTOR);

    return block_write(device, sectors_per_block, block, buf);
}


static int num_per_indirect(int block_size)
{
    return block_size / sizeof(block_no_t);
}
