#ifndef __FULFS_BLOCK__
#define __FULFS_BLOCK__

#include <stdint.h>
#include <stdbool.h>
#include "../device_io.h"

typedef uint32_t block_no_t;

static inline bool block_read(device_handle_t device, block_no_t no, int sectors_per_block, char* buf)
{
    return DEVICE_IO_SUCCESS(device_read(device, no * sectors_per_block, sectors_per_block, buf));
}

static inline bool block_write(device_handle_t device, block_no_t no, int sectors_per_block, const char* buf)
{

    return DEVICE_IO_SUCCESS(device_write(device, no * sectors_per_block, sectors_per_block, buf));
}



#endif /* __FULFS_BLOCK__ */
