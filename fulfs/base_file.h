#ifndef __FULFS__BASE__FILE__H__
#define __FULFS__BASE__FILE__H__

/* 本文件系统底层中一切皆文件，这里是底层的文件 */

#include "inode.h"

typedef struct {
    dev_inode_ctrl_t dev_inode_ctrl;
    inode_no_t inode_no;
    inode_t inode;

    /* 储存内部指针的底层信息 */
    struct {
        block_no_t current_block_relative;
        block_no_t current_block;
        int current_offset;
    }current;
}base_file_t;

bool base_file_open(base_file_t* base_file, dev_inode_ctrl_t* dev_inode_ctrl, inode_no_t inode_no); /* 打开底层文件 */

int base_file_mode(const base_file_t* base_file); /* 文件类型 */
fsize_t base_file_size(const base_file_t* base_file); /* 文件大小 */

/* 时间属性 */
time_t base_file_accessed_time(const base_file_t* base_file);
time_t base_file_modified_time(const base_file_t* base_file);
time_t base_file_created_time(const base_file_t* base_file);

/* IO */
bool base_file_seek(base_file_t* base_file, fsize_t offset);
bool base_file_read(base_file_t* base_file, int count, char* buf);
bool base_file_write(base_file_t* base_file, int count, const char* buf);

bool base_file_close(base_file_t* base_file);




#endif /* __FULFS__BASE__FILE__H__ */
