#ifndef __FULFS__FILESYSETM__H__
#define __FULFS__FILESYSETM__H__

#include <stdint.h>
#include "../device_io.h"
/*
  这里是关于unix like filesystem的高层操作。
  现在我决定把这个文件系统命名为fulfs ^_^
*/


typedef uint32_t block_no_t;

typedef enum {
    FULFS_SUCCESS,
    FULFS_FAIL
    /* 未来可以添加更多错误信息 */
}fulfs_errcode_t;

/* 将设备device格式化 */
fulfs_errcode_t fulfs_format(device_handle_t device, int sectors_per_block);




#endif /* __FULFS__FILESYSETM__H__ */
