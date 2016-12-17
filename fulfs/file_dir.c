#include "file_dir.h"


bool fulfs_open(fulfs_file_t* file, device_handle_t device, fulfs_filesystem_t* fs, const char* path)
{
    return true;
}
void fulfs_close(fulfs_file_t* file)
{

}

int fulfs_read(fulfs_file_t* file, char* buf, int count)
{
    return FS_ERROR;
}

int fulfs_write(fulfs_file_t* file, const char* buf, int count)
{
    return FS_ERROR;
}

int fulfs_ftruncate(fulfs_file_t* file, const char* buf, int count)
{
    return FS_ERROR;
}

fs_off_t fulfs_lseek(fulfs_file_t* file, fs_off_t off, int where)
{
    return FS_ERROR;
}

bool fulfs_mkdir(device_handle_t device, fulfs_filesystem_t* fs, const char* path)
{
    return false;
}

bool fulfs_rmdir(device_handle_t device, fulfs_filesystem_t* fs, const char* path)
{
    return false;
}

bool fulfs_link(device_handle_t device, fulfs_filesystem_t* fs, const char* src_path, const char* new_path)
{
    return false;
}

bool fulfs_unlink(device_handle_t device, fulfs_filesystem_t* fs, const char* path)
{
    return false;
}

bool fulfs_symlink(device_handle_t device, fulfs_filesystem_t* fs, const char* src_path, const char* new_path)
{
    return false;
}

bool fulfs_readlink(device_handle_t device, fulfs_filesystem_t* fs, const char *path, char *buf, size_t size)
{
    return false;
}


