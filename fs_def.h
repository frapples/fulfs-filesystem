#ifndef __FS__DEF__H__
#define __FS__DEF__H__

#include "stdint.h"

/* 这里面定义的函数接口，所有的文件系统应该要遵循此接口 */

typedef int64_t fs_off_t;

#define FS_ERROR -1

enum {
    FS_SEEK_SET,
    FS_SEEK_CUR,
    FS_SEEK_END
};

#endif /* __FS__DEF__H__ */
