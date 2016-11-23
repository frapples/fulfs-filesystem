#ifndef __FS_SUPERBLOCK__H__
#define __FS_SUPERBLOCK__H__

#include <stdint.h>
#include <stddef.h>



/*
  superblock储存在磁盘的头部，记录了文件系统总体的信息。
 */

/* 这个类型能容纳下一切的和文件系统相关的大小数据 */
typedef uint64_t fssize_t;

typedef struct {
    uint16_t sectors_per_block; /* block 大小 */
    uint16_t size_per_sector; /* 扇区大小 */
    uint64_t total_size; /* 文件系统总大小 */
    uint64_t used_size; /* 被使用的大小 */
    uint16_t root_dir; /* 指向根目录的 inode */
    uint32_t inode_table_block; /* inode table 起始的 block 号 */
    uint32_t data_block_free_stack; /* data block 的空闲管理相关 */
}superblock_t;

/* superblock的二进制表示的大小 */
size_t superblock_bin_size(void);

/* 将superblock转成二进制字节。bin的剩余部分会被填0，返回bin */
void superblock_dump(superblock_t* sb, char* bin, size_t bin_size);
/* 将二进制字节转换成superblock。bin太小了概不负责。返回sb */
void superblock_load(const char* bin, superblock_t* sb);



#endif /* __FS_SUPERBLOCK__H__ */
