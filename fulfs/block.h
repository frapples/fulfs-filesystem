#ifndef __FULFS_BLOCK__
#define __FULFS_BLOCK__

#include <stdint.h>
#include <stdbool.h>
#include "../device_io.h"


#define MAX_BYTES_PER_BLOCK (8 * 2 * BYTES_PER_SECTOR)

typedef uint32_t block_no_t;

static inline bool block_read(device_handle_t device, int sectors_per_block, block_no_t no, char* buf)
{
    return DEVICE_IO_SUCCESS(device_read(device, no * sectors_per_block, sectors_per_block, buf));
}

static inline bool block_write(device_handle_t device, int sectors_per_block, block_no_t no, const char* buf)
{

    return DEVICE_IO_SUCCESS(device_write(device, no * sectors_per_block, sectors_per_block, buf));
}



#endif /* __FULFS_BLOCK__ */
