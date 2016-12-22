#ifndef __FULFS__FILESYSETM__H__
#define __FULFS__FILESYSETM__H__

#include <stdint.h>
#include "superblock.h"
#include "block.h"
#include "../device_io.h"
#include "../fs_def.h"
/*
  这里是关于unix like filesystem的高层操作。
  现在我决定把这个文件系统命名为fulfs ^_^
*/



typedef enum {
    FULFS_SUCCESS,
    FULFS_FAIL
    /* 未来可以添加更多错误信息 */
}fulfs_errcode_t;

typedef struct {
    superblock_t sb;
}fulfs_filesystem_t;


fulfs_filesystem_t* fulfs_filesystem_new(device_handle_t device);
bool fulfs_filesystem_init(fulfs_filesystem_t* fs, device_handle_t device);

fs_size_t fulfs_filesystem_used_size(fulfs_filesystem_t* fs);
fs_size_t fulfs_filesystem_total_size(fulfs_filesystem_t* fs);

/* 将设备device格式化 */
bool fulfs_format(device_handle_t device, int sectors_per_block);





#endif /* __FULFS__FILESYSETM__H__ */
