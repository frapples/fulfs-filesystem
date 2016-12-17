#include "file_dir.h"

#include "base_file.h"
#include <string.h>
#include <assert.h>

static bool dir_locate(device_handle_t device, fulfs_filesystem_t* fs, inode_no_t dir, const char* name, bool* p_exist, inode_no_t* p_no);
static bool dir_add(device_handle_t device, fulfs_filesystem_t* fs, inode_no_t dir, const char* name, inode_no_t no);
static bool dir_del(device_handle_t device, fulfs_filesystem_t* fs, inode_no_t dir, const char* name);

bool fulfs_open(fulfs_file_t* file, device_handle_t device, fulfs_filesystem_t* fs, const char* path)
{
    /* TODO */
    return false;
}

void fulfs_close(fulfs_file_t* file)
{
    /* TODO */
}

int fulfs_read(fulfs_file_t* file, char* buf, int count)
{
    return base_file_read(&file->base_file, buf, count);
}

int fulfs_write(fulfs_file_t* file, const char* buf, int count)
{
    return base_file_write(&file->base_file, buf, count);
}

bool fulfs_ftruncate(fulfs_file_t* file, fsize_t size)
{
    return base_file_truncate(&file->base_file, size);
}

fs_off_t fulfs_lseek(fulfs_file_t* file, fs_off_t off, int where)
{
    fsize_t new_off;
    if (where == FS_SEEK_SET) {
        new_off = off;
    } else if (where == FS_SEEK_CUR) {
        new_off = base_file_tell(&file->base_file) + off;
    } else if (where == FS_SEEK_END) {
        new_off = base_file_size(&file->base_file) + off;
    } else {
        return FS_ERROR;
    }

    if (new_off > base_file_size(&file->base_file)) {
        return FS_ERROR;
    }

    base_file_seek(&file->base_file, off);

    return new_off;
}

bool fulfs_mkdir(device_handle_t device, fulfs_filesystem_t* fs, const char* path)
{
    /* TODO */
    return false;
}

bool fulfs_rmdir(device_handle_t device, fulfs_filesystem_t* fs, const char* path)
{
    /* TODO */
    return false;
}

bool fulfs_link(device_handle_t device, fulfs_filesystem_t* fs, const char* src_path, const char* new_path)
{
    /* TODO */
    return false;
}

bool fulfs_unlink(device_handle_t device, fulfs_filesystem_t* fs, const char* path)
{
    /* TODO */
    return false;
}

bool fulfs_symlink(device_handle_t device, fulfs_filesystem_t* fs, const char* src_path, const char* new_path)
{
    /* TODO */
    return false;
}

bool fulfs_readlink(device_handle_t device, fulfs_filesystem_t* fs, const char *path, char *buf, size_t size)
{
    /* TODO */
    return false;
}


/*************************************/
#define DIR_ITEM_NAME_SIZE 14
#define DIR_ITEM_SIZE (DIR_ITEM_NAME_SIZE + sizeof(inode_no_t))

struct dir_item_s {
    char name[DIR_ITEM_NAME_SIZE + 1];
    block_no_t inode_no;
};

static void dir_item_load_from_bin(struct dir_item_s* item, const char* bin);
static void dir_item_dump_to_bin(const struct dir_item_s* item, char* bin);

static bool dir_locate(device_handle_t device, fulfs_filesystem_t* fs, inode_no_t dir, const char* name, bool* p_exist, inode_no_t* p_no)
{
    base_file_t base_file;
    bool success = base_file_open(&base_file, device, &fs->sb, dir);
    if (!success) {
        return false;
    }

    assert(base_file_size(&base_file) % DIR_ITEM_SIZE == 0);

    char buf[DIR_ITEM_SIZE];
    while (base_file_tell(&base_file) < base_file_size(&base_file)) {
        int count = base_file_read(&base_file, buf, DIR_ITEM_SIZE);
        if (count != DIR_ITEM_SIZE) {
            base_file_close(&base_file);
            return false;
        }

        struct dir_item_s dir_item;
        dir_item_load_from_bin(&dir_item, buf);
        if (strcmp(dir_item.name, name) == 0) {
            *p_exist = true;
            *p_no = dir_item.inode_no;
            base_file_close(&base_file);
            return true;
        }
    }

    base_file_close(&base_file);
    *p_exist = false;
    return true;
}

static bool dir_add(device_handle_t device, fulfs_filesystem_t* fs, inode_no_t dir, const char* name, inode_no_t no)
{
    base_file_t base_file;
    bool success = base_file_open(&base_file, device, &fs->sb, dir);
    if (!success) {
        return false;
    }

    assert(base_file_size(&base_file) % DIR_ITEM_SIZE == 0);

    char buf[DIR_ITEM_SIZE];
    struct dir_item_s dir_item;
    strncpy(dir_item.name, name, DIR_ITEM_NAME_SIZE);
    dir_item.name[DIR_ITEM_NAME_SIZE] = '\0';
    dir_item_dump_to_bin(&dir_item, buf);
    dir_item.inode_no = no;

    base_file_seek(&base_file, base_file_size(&base_file));

    /* FIXME: 这里写入不完整的话，应该把文件截断到之前的状态 */
    int count = base_file_write(&base_file, buf, DIR_ITEM_SIZE);
    if (count != DIR_ITEM_SIZE) {
        base_file_close(&base_file);
        return false;
    }

    base_file_close(&base_file);
    return true;
}

static bool dir_del(device_handle_t device, fulfs_filesystem_t* fs, inode_no_t dir, const char* name)
{
    base_file_t base_file;
    bool success = base_file_open(&base_file, device, &fs->sb, dir);
    if (!success) {
        return false;
    }

    assert(base_file_size(&base_file) % DIR_ITEM_SIZE == 0);

    char buf[DIR_ITEM_SIZE];
    while (base_file_tell(&base_file) < base_file_size(&base_file)) {
        int count = base_file_read(&base_file, buf, DIR_ITEM_SIZE);
        if (count != DIR_ITEM_SIZE) {
            base_file_close(&base_file);
            return false;
        }

        struct dir_item_s dir_item;
        dir_item_load_from_bin(&dir_item, buf);
        if (strcmp(dir_item.name, name) == 0) {
            /* 把最后一项读出来覆盖欲删除的项 */
            fsize_t current = base_file_tell(&base_file) - DIR_ITEM_SIZE;

            base_file_seek(&base_file, base_file_size(&base_file) - DIR_ITEM_SIZE);
            int count = base_file_read(&base_file, buf, DIR_ITEM_SIZE);
            if (count != DIR_ITEM_SIZE) {
                base_file_close(&base_file);
                return false;
            }

            base_file_seek(&base_file, current);
            count = base_file_write(&base_file, buf, DIR_ITEM_SIZE);
            if (count != DIR_ITEM_SIZE) {
                base_file_close(&base_file);
                return false;
            }

            /* FIXME: 这里要是失败了，又是个很尴尬的问题，会导致这个目录里出现两个相同的项 */
            bool success = base_file_truncate(&base_file, base_file_size(&base_file) - DIR_ITEM_SIZE);
            if (!success) {
                base_file_close(&base_file);
                return false;
            }

            base_file_close(&base_file);
            return true;
        }
    }

    base_file_close(&base_file);
    return true;
}



/*************************************/

/* 简单实现，考虑了内存布局但是未考虑字节序 */
static void dir_item_load_from_bin(struct dir_item_s* item, const char* bin)
{
    /* 二进制的数据里面，文件名称在达到14个字节的情况下，允许不带\0 */
    memcpy(&item->inode_no, bin, sizeof(inode_no_t));
    memcpy(item->name, bin  + sizeof(inode_no_t), DIR_ITEM_NAME_SIZE);
    item->name[DIR_ITEM_NAME_SIZE] = '\0';
}

static void dir_item_dump_to_bin(const struct dir_item_s* item, char* bin)
{
    memcpy(bin, &item->inode_no, sizeof(inode_no_t));
    memcpy(bin  + sizeof(inode_no_t), item->name, DIR_ITEM_NAME_SIZE);
}
