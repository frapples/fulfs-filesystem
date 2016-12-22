#ifndef __FULFS_SUPERBLOCK__H__
#define __FULFS_SUPERBLOCK__H__

#include <stdint.h>
#include <stddef.h>

#include "../device_io.h"
#include "block.h"
#include "def.h"



/*
  superblock储存在磁盘的头部，记录了文件系统总体的信息。
 */

/* 这个类型能容纳下一切的和文件系统相关的大小数据 */
typedef uint64_t fssize_t;

typedef struct {
    uint16_t sectors_per_block; /* 每个block占的扇区数 */
    sector_no_t sectors; /* 总的扇区数 */
    uint64_t total_size; /* 文件系统总大小 */
    // uint64_t used_size; /* 被使用的大小 */
    inode_no_t root_dir; /* 指向根目录的 inode */
    block_no_t inode_table_block; /* inode table 起始的 block 号 */
    inode_no_t inode_total_count; /* inode table区总inode个数 */
    block_no_t data_block; /* data区的起始block号 */
    block_no_t data_block_free_stack; /* data block 的空闲管理相关 */

    inode_no_t used_inode_count; /* 已经使用的inode节点个数 注：在base_file层，删除文件减1，创建文件加1 */
    block_no_t used_data_block_count; /* 已经使用的data block块个数  注：在base_block_file层负责维护此变量 */
}superblock_t;

/* superblock的二进制表示的大小 */
size_t superblock_bin_size(void);

/* 将superblock转成二进制字节。bin的剩余部分会被填0，返回bin */
void superblock_dump_to_bin(superblock_t* sb, char* bin, size_t bin_size);
/* 将二进制字节转换成superblock。bin太小了概不负责。返回sb */
void superblock_load_from_bin(const char* bin, superblock_t* sb);

/* 给一个新的文件系统初始化superblock */
void superblock_create(superblock_t* sb,  sector_no_t sectors, int sectors_per_block,
                       block_no_t inode_table, inode_no_t inode_count, block_no_t data_block, block_no_t data_block_free_stack, inode_no_t root_inode);


/* 从磁盘加载superblock */
bool superblock_load(device_handle_t device, superblock_t* sb);
/* 将superblock写入磁盘 */
bool superblock_dump(device_handle_t device, superblock_t* sb);

/* 文件系统block总数 */
block_no_t superblock_block_count(const superblock_t* sb);
size_t superblock_block_size(const superblock_t* sb); /* 每个block的大小 */
int superblock_sectors_per_block(const superblock_t *sb);

uint64_t superblock_total_size(const superblock_t* sb); /* 文件系统总大小 */
uint64_t superblock_used_size(const superblock_t* sb); /* 文件系统已用大小 */
uint64_t superblock_free_size(const superblock_t* sb); /* 文件系统空闲大小 */
void superblock_set_used_size(superblock_t* sb, uint64_t new_size); /* 设置已使用的空间大小 */
inode_no_t superblock_root_dir_inode(const superblock_t* sb); /* 根目录inode */


block_no_t superblock_inode_table_start(const superblock_t* sb); /* inode起始block */
block_no_t superblock_inode_table_blocksize(const superblock_t* sb); /* inode 占用的block数 */

block_no_t superblock_data_block_start(const superblock_t* sb); /* data起始block */
block_no_t superblock_data_block_size(const superblock_t* sb); /* data占用的block数 */

block_no_t superblock_data_block_free_stack(const superblock_t* sb); /* free空闲栈管理相关 */

void superblock_data_block_free_stack_set(superblock_t* sb, block_no_t new_stack);


#endif /* __FULFS_SUPERBLOCK__H__ */
