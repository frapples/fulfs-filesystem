#ifndef __FULFS_BLOCK__
#define __FULFS_BLOCK__

#include <stdint.h>
#include <stdbool.h>
#include "../device_io.h"
#include "../utils/log.h"


#define MAX_BYTES_PER_BLOCK (8 * 2 * BYTES_PER_SECTOR)

typedef uint32_t block_no_t;

static inline bool block_read(device_handle_t device, int sectors_per_block, block_no_t no, char* buf)
{

    bool success = DEVICE_IO_SUCCESS(device_read(device, no * sectors_per_block, sectors_per_block, buf));

    if (!success) {
        log_debug("读取block失败: %d号设备, 块号%ld\n", device, no);
    }
    return success;
}

static inline bool block_write(device_handle_t device, int sectors_per_block, block_no_t no, const char* buf)
{
    if (no == 0) {
        log_warning("写入%d号设备的0号块（可能是superblock所在块）\n", device);
    }

    bool success = DEVICE_IO_SUCCESS(device_write(device, no * sectors_per_block, sectors_per_block, buf));

    if (!success) {
        log_debug("写入block失败: %d号设备, 块号%ld\n", device, no);
    }
    return success;
}

static inline bool block_copy(device_handle_t device, int sectors_per_block, block_no_t from, block_no_t to)
{
    char buf[MAX_BYTES_PER_BLOCK];
    bool success = block_read(device, sectors_per_block, from, buf);
    if (!success) {
        return false;
    }

    return block_write(device, sectors_per_block, to, buf);
}



#endif /* __FULFS_BLOCK__ */
