
#include "data_block.h"

#include <stdint.h>
#include <stdbool.h>
#include "block.h"

#include <assert.h>

/* 使用成组链接法管理空闲的block */

#define MAX_GROUP_COUNT 100
struct group_s{
    uint8_t top;
    block_no_t free_block[MAX_GROUP_COUNT];
};

#define GROUP_NEXT_END ((block_no_t)0)

#define GROUP_NEXT(group) (group.free_block[0])

/* 同理，一个简单的实现，没有考虑字节序和内存布局 */
static void group_load(const char* buf, struct group_s* group) {
    *group = *((struct group_s*)buf);
}
static void group_dump(const struct group_s* group, char* buf) {
    *((struct group_s*)buf) = *group;
}


bool data_blocks_init(device_handle_t device, int sectors_per_block, block_no_t data_block_start, block_no_t data_block_count, block_no_t* p_start)
{
    block_no_t next = data_block_start;
    for (block_no_t block = data_block_start + 1; block < data_block_start + data_block_count; block += MAX_GROUP_COUNT) {
        bool is_end = (data_block_start + data_block_count - block) <= MAX_GROUP_COUNT;

        struct group_s group;
        group.top =  is_end ? (data_block_start + data_block_count - block) : MAX_GROUP_COUNT;

        assert(group.top <= MAX_GROUP_COUNT);

        for (int i = 0; i < group.top; i++) {
            group.free_block[i] = block + i;
        }

        if (is_end) {
            GROUP_NEXT(group) = GROUP_NEXT_END;
        }

        char buf[MAX_BYTES_PER_BLOCK];
        group_dump(&group, buf);

        assert(next >= data_block_start);

        bool success = block_write(device, sectors_per_block, next, buf);
        if (!success) {
            return false;
        }
        next = GROUP_NEXT(group);
    }
    *p_start = data_block_start;
    return true;
}

bool data_block_alloc(device_handle_t device, int sectors_per_block, block_no_t data_blocks_stack, block_no_t* p_block, block_no_t* p_used_block_count)
{
    char buf[MAX_BYTES_PER_BLOCK];
    block_read(device, sectors_per_block, data_blocks_stack, buf);

    struct group_s group;
    group_load(buf, &group);

    assert(group.top > 0);
    assert(group.top <= MAX_GROUP_COUNT);


    if (group.top == 1) {
        block_no_t next = GROUP_NEXT(group);
        if (next == GROUP_NEXT_END) {
            return false;
        }

        *p_block = group.free_block[--group.top];

        bool success = block_copy(device, sectors_per_block, next, data_blocks_stack);
        if (!success) {
            return false;
        }
    } else {
        *p_block = group.free_block[--group.top];

        group_dump(&group, buf);
        bool success = block_write(device, sectors_per_block, data_blocks_stack, buf);
        if (!success) {
            return false;
        }
    }
    *p_used_block_count += 1;
    return true;
}

bool data_block_free(device_handle_t device, int sectors_per_block, block_no_t data_blocks_stack, block_no_t block, block_no_t* p_used_block_count)
{
    char buf[MAX_BYTES_PER_BLOCK];
    block_read(device, sectors_per_block, data_blocks_stack, buf);

    struct group_s group;
    group_load(buf, &group);

    assert(group.top <= MAX_GROUP_COUNT);

    if (group.top == MAX_GROUP_COUNT) {
        bool success = block_copy(device, sectors_per_block, data_blocks_stack, block);
        if (!success) {
            return false;
        }

        group.top = 1;
        GROUP_NEXT(group) = block;
    } else {
        group.free_block[group.top++] = block;
    }

    group_dump(&group, buf);
    bool success = block_write(device, sectors_per_block, data_blocks_stack, buf);
    if (!success) {
        return false;
    }

    assert(p_used_block_count > 0);

    *p_used_block_count -= 1;
    return true;
}
