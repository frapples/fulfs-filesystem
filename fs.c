#include "fs.h"

#include "fs_def.h"

#include "fulfs/fulfs.h"


struct fs_operate_functions_s operate_functions[FS_TYPE_TOTAL];

void fs_init(void)
{
    FS_OPERATE_FUNCTIONS_SET(operate_functions[FS_TYPE_FULFS], fulfs);
}


int fs_open(const char* path)
{
    return FS_ERROR;
}
void fs_close(int fd)
{

}

int fs_read(int fd, char* buf, int count)
{
    
    return FS_ERROR;
}
int fs_write(int fd, const char* buf, int count)
{
    
    return FS_ERROR;
}
bool fs_ftruncate(int fd, fs_off_t size)
{
    return FS_ERROR;
    
}
fs_off_t fs_lseek(int fd, fs_off_t off, int where)
{
    
    return FS_ERROR;
}

int fs_mkdir(const char* path)
{
    return FS_ERROR;
    
}
int fs_rmdir(const char* path)
{
    return FS_ERROR;
}

int fs_link(const char* src_path, const char* new_path)
{
    return FS_ERROR;
}
int fs_unlink(const char* path)
{
    return FS_ERROR;
}

int fs_symlink(const char* src_path, const char* new_path)
{
    return FS_ERROR;
}
int fs_readlink(const char *path, char *buf, size_t size)
{
    return FS_ERROR;
}

int fs_stat(const char *path, struct fs_stat *buf)
{
    return FS_ERROR;
}



int fs_opendir(fs_dir_t* dir, const char *path)
{
    return FS_ERROR;
}
int fs_readdir(fs_dir_t* dir, char* name)
{
    return FS_ERROR;
}
void fs_closedir(fs_dir_t* dir)
{
}
