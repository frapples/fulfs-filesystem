#include "base_file.h"

#include "mem_inode.h"
#include "block.h"
#include "base_block_file.h"
#include "../utils/math.h"
#include "../utils/log.h"
#include <string.h>
#include <time.h>

#include <assert.h>


/* TODO 关于时间的部分没管它 */

static bool base_file_del(device_handle_t device, superblock_t* sb, inode_no_t inode_no);


bool base_file_open(base_file_t* base_file, device_handle_t device, superblock_t* sb, inode_no_t inode_no)
{
    base_file->device = device;
    base_file->inode_no = inode_no;
    dev_inode_ctrl_t dev_inode_ctrl;
    dev_inode_ctrl_init_from_superblock(&dev_inode_ctrl, device, sb);
    base_file->sb = sb;
    bool success = mem_inode_get(&dev_inode_ctrl, inode_no, &(base_file->mem_inode));
    if (!success) {
        return false;
    }

    base_file->current.current_block_relative = 0;
    base_file->current.current_offset = 0;

    base_file->mem_inode->inode.accessed_time = time(NULL);
    return true;
}

int base_file_mode(const base_file_t* base_file)
{
    return base_file->mem_inode->inode.mode;
}

fsize_t base_file_size(const base_file_t* base_file)
{
    return base_file->mem_inode->inode.size;
}

timestamp_t base_file_accessed_time(const base_file_t* base_file)
{
    return base_file->mem_inode->inode.accessed_time;
}

timestamp_t base_file_modified_time(const base_file_t* base_file)
{
    return base_file->mem_inode->inode.modified_time;
}

timestamp_t base_file_created_time(const base_file_t* base_file)
{
    return base_file->mem_inode->inode.created_time;
}

bool base_file_seek(base_file_t* base_file, fsize_t offset)
{
    assert(offset <= base_file->mem_inode->inode.size);

    block_no_t block_relative = offset / superblock_block_size(base_file->sb);
    base_file->current.current_block_relative = block_relative;
    base_file->current.current_offset = offset % superblock_block_size(base_file->sb);
    return true;
}

fsize_t base_file_tell(const base_file_t* base_file)
{
    return base_file->current.current_block_relative * superblock_block_size(base_file->sb)
        + base_file->current.current_offset;
}

int base_file_read(base_file_t* base_file, char* buf, int count)
{
    int sectors_per_block = superblock_sectors_per_block(base_file->sb);

    /* 保证接下来的count没有超过文件大小 */
    if (base_file_size(base_file) - base_file_tell(base_file) < (fsize_t)count) {
        count = base_file_size(base_file) - base_file_tell(base_file);
    }

    char block_buf[MAX_BYTES_PER_BLOCK];
    int readed_count = 0;
    while (readed_count < count) {
        block_no_t current_block;
        bool success = base_block_file_locate(base_file->device, base_file->sb, &(base_file->mem_inode->inode),
                                              base_file->current.current_block_relative, &current_block);
        if (!success) {
            log_debug("定位文件block失败: %d号设备, 文件inode号%d, 相对块号%d\n", base_file->device,
                      base_file->inode_no, base_file->current.current_block_relative);
            return readed_count;
        }

        success = block_read(base_file->device, sectors_per_block, current_block, block_buf);
        if (!success) {
            log_debug("读取块失败: %d号设备, 文件inode号%d, 块号%d\n", base_file->device, base_file->inode_no, current_block);
            return readed_count;
        }

        int should_read_size = min_int(sectors_per_block * BYTES_PER_SECTOR - base_file->current.current_offset, count - readed_count);

        memcpy(buf + readed_count, block_buf + base_file->current.current_offset, should_read_size);
        readed_count += should_read_size;

        base_file_seek(base_file, base_file_tell(base_file) + should_read_size);
    }

    return readed_count;
}

int base_file_write(base_file_t* base_file, const char* buf, int count)
{
    int sectors_per_block = superblock_sectors_per_block(base_file->sb);

    char block_buf[MAX_BYTES_PER_BLOCK];
    int writed_count = 0;
    while (writed_count < count) {

        block_no_t current_block;
        if (base_file->current.current_block_relative >= count_groups(base_file_size(base_file), sectors_per_block * BYTES_PER_SECTOR)) {
            bool success = base_block_file_push_block(base_file->device, base_file->sb, &base_file->mem_inode->inode, &current_block);
            if (!success) {
                log_debug("分配新block失败: %d号设备, 文件inode号%d, 相对块号%d\n", base_file->device,
                          base_file->inode_no, base_file->current.current_block_relative);
                return writed_count;
            }
        } else {
            bool success = base_block_file_locate(base_file->device, base_file->sb, &base_file->mem_inode->inode,
                                                  base_file->current.current_block_relative, &current_block);
            if (!success) {
                log_debug("定位文件block失败: %d号设备, 文件inode号%d, 相对块号%d\n", base_file->device,
                          base_file->inode_no, base_file->current.current_block_relative);
                return writed_count;
            }
        }

        bool success = block_read(base_file->device, sectors_per_block, current_block, block_buf);
        if (!success) {
            return writed_count;
        }

        int should_write_size = min_int(sectors_per_block * BYTES_PER_SECTOR - base_file->current.current_offset,
                                       count - writed_count);

        memcpy(block_buf + base_file->current.current_offset, buf + writed_count, should_write_size);

        success = block_write(base_file->device, sectors_per_block, current_block, block_buf);
        if (!success) {
            return writed_count;
        }

        writed_count += should_write_size;

        fsize_t will_pos = base_file_tell(base_file) + should_write_size;
        if (will_pos > base_file_size(base_file)) {
            base_file->mem_inode->inode.size = will_pos;
        }
        base_file_seek(base_file, will_pos);
    }

    base_file->mem_inode->inode.modified_time = time(NULL);
    return writed_count;
}

bool base_file_close(base_file_t* base_file)
{
    dev_inode_ctrl_t dev_inode_ctrl;
    dev_inode_ctrl_init_from_superblock(&dev_inode_ctrl, base_file->device, base_file->sb);

    if (!superblock_dump(base_file->device, base_file->sb)) {
        return false;
    }

    return mem_inode_put(&dev_inode_ctrl, base_file->mem_inode);
}

bool base_file_create(device_handle_t device, superblock_t* sb, int mode, inode_no_t* p_inode_no)
{
    dev_inode_ctrl_t dev_inode_ctrl;
    dev_inode_ctrl_init_from_superblock(&dev_inode_ctrl, device, sb);

    bool success = inode_alloc(&dev_inode_ctrl, p_inode_no);
    if (!success) {
        return false;
    }

    inode_t inode;
    success = inode_load(&dev_inode_ctrl, *p_inode_no, &inode);
    if (!success) {
        return false;
    }

    inode.mode = mode;
    inode.size = 0;
    inode.link_count = 1;

    inode.accessed_time = time(NULL);
    inode.modified_time = time(NULL);
    inode.created_time = time(NULL);

    sb->used_inode_count++;

    if (!superblock_dump(device, sb)) {
        return false;
    }
    success = inode_dump(&dev_inode_ctrl, *p_inode_no, &inode);
    if (!success) {
        return false;
    }

    return true;
}

bool base_file_ref(device_handle_t device, superblock_t* sb, inode_no_t inode_no)
{
    dev_inode_ctrl_t dev_inode_ctrl;
    dev_inode_ctrl_init_from_superblock(&dev_inode_ctrl, device, sb);

    mem_inode_t* mem_inode;
    bool success = mem_inode_get(&dev_inode_ctrl, inode_no, &mem_inode);
    if (!success) {
        return false;
    }

    mem_inode->inode.link_count++;

    return mem_inode_put(&dev_inode_ctrl, mem_inode);
}

bool base_file_unref(device_handle_t device, superblock_t* sb, inode_no_t inode_no)
{
    dev_inode_ctrl_t dev_inode_ctrl;
    dev_inode_ctrl_init_from_superblock(&dev_inode_ctrl, device, sb);

    mem_inode_t* mem_inode;
    bool success = mem_inode_get(&dev_inode_ctrl, inode_no, &mem_inode);
    if (!success) {
        return false;
    }

    mem_inode->inode.link_count--;
    if (mem_inode->inode.link_count > 0) {
        return mem_inode_put(&dev_inode_ctrl, mem_inode);
    } else {
        return base_file_del(device, sb, inode_no);
    }
}

bool base_file_truncate(base_file_t* base_file, fsize_t size)
{
    /* 这个函数要保证出错时不破坏完整性 */
    if (base_file->mem_inode->inode.size > size) {

        int block_size = superblock_block_size(base_file->sb);

        block_no_t block_num = count_groups(base_file->mem_inode->inode.size, block_size);
        block_no_t should_block_num = count_groups(size, block_size);

        for (block_no_t i = 0; i < block_num - should_block_num; i++) {

            /* 那么，完整性的责任转由这个函数保证 */
            bool success = base_block_file_pop_block(base_file->device, base_file->sb, &(base_file->mem_inode->inode));
            if (!success) {
                return false;
            }

            base_file->mem_inode->inode.size -= block_size;
        }

        base_file->mem_inode->inode.size = size;

        base_file->mem_inode->inode.modified_time = time(NULL);
        return true;

    } else {
        return true;
    }
}


int base_file_ref_count(base_file_t* base_file)
{
    return base_file->mem_inode->inode.link_count;
}

bool base_file_block_count(base_file_t* base_file, long* p_count)
{
    return base_block_file_block_count(base_file->device, base_file->sb, &base_file->mem_inode->inode, p_count);
}

/*********************************/
static bool base_file_del(device_handle_t device, superblock_t* sb, inode_no_t inode_no)
{
    base_file_t base_file;
    bool success = base_file_open(&base_file, device, sb, inode_no);
    if (!success) {
        return false;
    }

    bool has_fp = base_file.mem_inode->ref_count > 0;

    /* 释放block */
    success = base_file_truncate(&base_file, 0);
    if (!success) {
        return false;
    }
    base_file_close(&base_file);

    if (!has_fp) {
        return false;
    }


    /* 释放inode */
    dev_inode_ctrl_t dev_inode_ctrl;
    dev_inode_ctrl_init_from_superblock(&dev_inode_ctrl, device, sb);
    inode_free(&dev_inode_ctrl, inode_no);

    sb->used_inode_count--;
    return true;
}

