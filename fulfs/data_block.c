
#include "data_block.h"

#include <stdint.h>
#include <stdbool.h>
#include "block.h"

/* 使用成组链接法管理空闲的block */

#define MAX_GROUP_COUNT 100
struct group_s{
    int8_t top;
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


block_no_t data_blocks_init(device_handle_t device, int sectors_per_block, block_no_t data_block_start, block_no_t data_block_count)
{
    block_no_t next = data_block_start;
    for (block_no_t block = data_block_start + 1; block < data_block_start + data_block_count; block += MAX_GROUP_COUNT) {
        bool is_end = (data_block_start + data_block_count - block) > MAX_GROUP_COUNT;

        struct group_s group;
        group.top =  is_end ? MAX_GROUP_COUNT : (data_block_start + data_block_count - block);

        for (int i = 0; i < group.top; i++) {
            group.free_block[i] = block + i;
        }

        if (is_end) {
            GROUP_NEXT(group) = GROUP_NEXT_END;
        }

        char buf[MAX_BYTES_PER_BLOCK];
        group_dump(&group, buf);
        block_write(device, sectors_per_block, next, buf);
        next = GROUP_NEXT(group);
    }
    return data_block_start;
}
