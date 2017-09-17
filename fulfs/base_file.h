#ifndef __FULFS__BASE__FILE__H__
#define __FULFS__BASE__FILE__H__

/* 本文件系统底层中一切皆文件，这里是底层的文件 */

#include "mem_inode.h"
#include "superblock.h"

typedef struct {
    device_handle_t device;
    inode_no_t inode_no;
    mem_inode_t* mem_inode;
    superblock_t* sb;

    /* 储存内部指针的底层信息 */
    struct {
        block_no_t current_block_relative;
        int current_offset;
    }current;
}base_file_t;

/* 打开底层文件, 注意：spuerblock的内存使用由用户决定 */
bool base_file_open(base_file_t* base_file, device_handle_t device, superblock_t* sb, inode_no_t inode_no);

int base_file_mode(const base_file_t* base_file); /* 文件类型 */
fsize_t base_file_size(const base_file_t* base_file); /* 文件大小 */

/* 时间属性 */
timestamp_t base_file_accessed_time(const base_file_t* base_file);
timestamp_t base_file_modified_time(const base_file_t* base_file);
timestamp_t base_file_created_time(const base_file_t* base_file);

/* IO */
bool base_file_seek(base_file_t* base_file, fsize_t offset);
fsize_t base_file_tell(const base_file_t* base_file);

#define BASE_FILE_IO_ERROR -1
int base_file_read(base_file_t* base_file, char* buf, int count);
int base_file_write(base_file_t* base_file, const char* buf, int count);

bool base_file_truncate(base_file_t* base_file, fsize_t size); /* 把文件内容截断 */

bool base_file_close(base_file_t* base_file);

int base_file_ref_count(base_file_t* base_file);
bool base_file_block_count(base_file_t* base_file, long* count);

bool base_file_create(device_handle_t device, superblock_t* sb, int mode, inode_no_t* p_inode_no); /* 创建底层文件 */

bool base_file_ref(device_handle_t device, superblock_t* sb, inode_no_t inode_no); /* 文件的引用加1 */
bool base_file_unref(device_handle_t device, superblock_t* sb, inode_no_t inode_no); /* 文件引用减1，到0就删掉 */




#endif /* __FULFS__BASE__FILE__H__ */
