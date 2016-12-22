#ifndef __FS__DEF__H__
#define __FS__DEF__H__

#include "device_io.h"

#include <time.h>
#include <stdint.h>
#include <stdbool.h>

/* 这里面定义的函数接口，所有的文件系统应该要遵循此接口 */

typedef int64_t fs_off_t;
typedef int64_t fs_size_t;
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
    long      st_ino;
    int       st_nlink;     //连到该文件的硬连接数目，刚建立的文件值为1
    int st_mode;
    fs_off_t         st_size;      //文件字节数(文件大小)
    unsigned long st_blksize;   //块大小(文件系统的I/O 缓冲区大小)
    unsigned long st_blocks;    //块数
    time_t        st_atime;     //最后一次访问时间
    time_t        st_mtime;     //最后一次修改时间
    time_t        st_ctime;     //最后一次改变时间(指属性)
};



/** 实现的某种特定的文件系统必须要先实现以下的接口 */
typedef void fs_filesystem_t;

typedef fs_filesystem_t* (fs_filesystem_new_f)(device_handle_t device);

typedef void fs_file_t;
typedef void fs_dir_t;

typedef fs_file_t* (fs_open_f)(device_handle_t device, fs_filesystem_t* fs, const char* path);
typedef void     (fs_close_f)(fs_file_t* file);

typedef int      (fs_read_f)(fs_file_t* file, char* buf, int count);

typedef int      (fs_write_f)(fs_file_t* file, const char* buf, int count);
typedef bool     (fs_ftruncate_f)(fs_file_t* file, fs_off_t size);
typedef fs_off_t (fs_lseek_f)(fs_file_t* file, fs_off_t off, int where);

typedef bool (fs_mkdir_f)(device_handle_t device, fs_filesystem_t* fs, const char* path);
typedef bool (fs_rmdir_f)(device_handle_t device, fs_filesystem_t* fs, const char* path);

typedef bool (fs_link_f)(device_handle_t device, fs_filesystem_t* fs, const char* src_path, const char* new_path);
typedef bool (fs_unlink_f)(device_handle_t device, fs_filesystem_t* fs, const char* path);

typedef bool (fs_symlink_f)(device_handle_t device, fs_filesystem_t* fs, const char* src_path, const char* new_path);
typedef bool (fs_readlink_f)(device_handle_t device, fs_filesystem_t* fs, const char *path, char *buf, size_t size);

typedef bool (fs_stat_f)(device_handle_t device, fs_filesystem_t* fs, const char *path, struct fs_stat *buf);

typedef fs_dir_t* (fs_opendir_f)(device_handle_t device, fs_filesystem_t* fs, const char *path);
typedef bool (fs_readdir_f)(fs_dir_t* dir, char* name);
typedef bool (fs_closedir_f)(fs_dir_t* dir);

/* 文件系统本身操作 */
typedef bool (fs_format_f)(device_handle_t device, int sectors_per_block);

typedef fs_size_t (fs_filesystem_used_size_f)(fs_filesystem_t* fs);
typedef fs_size_t (fs_filesystem_total_size_f)(fs_filesystem_t* fs);

/* 类似虚表的一个东西 */

struct fs_operate_functions_s{
    fs_filesystem_new_f* filesystem_new;

    fs_open_f* open;
    fs_close_f* close;
    fs_read_f* read;
    fs_write_f* write;
    fs_ftruncate_f* ftruncate;
    fs_lseek_f* lseek;
    fs_mkdir_f* mkdir;
    fs_rmdir_f* rmdir;

    fs_link_f* link;
    fs_unlink_f* unlink;

    fs_symlink_f* symlink;
    fs_readlink_f* readlink;
    fs_stat_f* stat;
    fs_opendir_f* opendir;
    fs_readdir_f* readdir;
    fs_closedir_f* closedir;

    fs_format_f* format;
    fs_filesystem_total_size_f* filesystem_total_size;
    fs_filesystem_used_size_f* filesystem_used_size;
};

#define FS_OPERATE_FUNCTIONS_SET(var, type_name)                        \
    do {                                                                \
        (var).open = (fs_open_f*)type_name##_open;                      \
        (var).close = (fs_close_f*)type_name##_close;                   \
        (var).read = (fs_read_f*)type_name##_read;                      \
        (var).write = (fs_write_f*)type_name##_write;                   \
        (var).ftruncate = (fs_ftruncate_f*)type_name##_ftruncate;       \
        (var).lseek = (fs_lseek_f*)type_name##_lseek;                   \
        (var).mkdir = (fs_mkdir_f*)type_name##_mkdir;                   \
        (var).rmdir = (fs_rmdir_f*)type_name##_rmdir;                   \
        (var).link = (fs_link_f*)type_name##_link;                      \
        (var).unlink = (fs_unlink_f*)type_name##_unlink;                \
        (var).symlink = (fs_symlink_f*)type_name##_symlink;             \
        (var).readlink = (fs_readlink_f*)type_name##_readlink;          \
        (var).stat = (fs_stat_f*)type_name##_stat;                      \
        (var).opendir = (fs_opendir_f*)type_name##_opendir;             \
        (var).readdir = (fs_readdir_f*)type_name##_readdir;             \
        (var).closedir = (fs_closedir_f*)type_name##_closedir;          \
        (var).filesystem_new = (fs_filesystem_new_f*)type_name##_filesystem_new; \
        (var).format = (fs_format_f*)type_name##_format;                \
        (var).filesystem_total_size = (fs_filesystem_total_size_f*)type_name##_filesystem_total_size; \
        (var).filesystem_used_size = (fs_filesystem_used_size_f*)type_name##_filesystem_used_size; \
                                                                        \
    } while(0);                                                         \

#endif /* __FS__DEF__H__ */
