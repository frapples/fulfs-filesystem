#ifndef __FULFS__FILE_DIR__H__
#define __FULFS__FILE_DIR__H__

#include "base_file.h"
#include "filesystem.h"

#include "../fs_def.h"

typedef struct  {
    base_file_t base_file;
}fulfs_file_t;

bool fulfs_open(fulfs_file_t* file, device_handle_t device, fulfs_filesystem_t* fs, const char* path);
void fulfs_close(fulfs_file_t* file);

int fulfs_read(fulfs_file_t* file, char* buf, int count);
int fulfs_write(fulfs_file_t* file, const char* buf, int count);
int fulfs_ftruncate(fulfs_file_t* file, const char* buf, int count);
fs_off_t fulfs_lseek(fulfs_file_t* file, fs_off_t off, int where);

bool fulfs_mkdir(device_handle_t device, fulfs_filesystem_t* fs, const char* path);
bool fulfs_rmdir(device_handle_t device, fulfs_filesystem_t* fs, const char* path);

bool fulfs_link(device_handle_t device, fulfs_filesystem_t* fs, const char* src_path, const char* new_path);
bool fulfs_unlink(device_handle_t device, fulfs_filesystem_t* fs, const char* path);

bool fulfs_symlink(device_handle_t device, fulfs_filesystem_t* fs, const char* src_path, const char* new_path);
bool fulfs_readlink(device_handle_t device, fulfs_filesystem_t* fs, const char *path, char *buf, size_t size);

#endif /* __FULFS__FILE_DIR__H__ */
