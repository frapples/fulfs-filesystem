#ifndef __FS__DEF__H__
#define __FS__DEF__H__

#include "stdint.h"
#include "time.h"

/* 这里面定义的函数接口，所有的文件系统应该要遵循此接口 */

typedef int64_t fs_off_t;
#define FS_MAX_FILE_PATH 2048

#define FS_ERROR -1
#define FS_MAX_PATH

enum {
    FS_SEEK_SET,
    FS_SEEK_CUR,
    FS_SEEK_END
};

#define FS_S_IFREG   (2 >> 0)
#define FS_S_IFDIR   (2 >> 1)
#define FS_S_IFLNK   (2 >> 2)

struct fs_stat {
    int       st_nlink;     //连到该文件的硬连接数目，刚建立的文件值为1
    int st_mode;
    fs_off_t         st_size;      //文件字节数(文件大小)
    unsigned long st_blksize;   //块大小(文件系统的I/O 缓冲区大小)
    unsigned long st_blocks;    //块数
    time_t        st_atime;     //最后一次访问时间
    time_t        st_mtime;     //最后一次修改时间
    time_t        st_ctime;     //最后一次改变时间(指属性)
};


#endif /* __FS__DEF__H__ */
