#include "fs.h"

#include "fs_def.h"
#include "fulfs/fulfs.h"
#include <ctype.h>


struct fs_operate_functions_s g_operate_functions[FS_TYPE_TOTAL];

/* 这个表记录了盘符到具体操作系统的记录 */
struct dev_fsctrl_s{
    int fs_type;
    struct fs_operate_functions_s* opfuncs;
    device_handle_t device;
    fs_filesystem_t* fs_ctrl;
};

struct dev_fsctrl_s g_device_filesystem['z' - 'a'];


void fs_init(void)
{
    FS_OPERATE_FUNCTIONS_SET(g_operate_functions[FS_TYPE_FULFS], fulfs);

    for (int i = 0; i < 'z' - 'a'; i++) {
        g_device_filesystem[i].fs_type = FS_TYPE_NULL;
    }
}


/************************/


bool fs_mount(device_handle_t device, char drive_letter, int fs_type)
{
    if (!('a' <= drive_letter && drive_letter <= 'z')) {
        return false;
    }

    if (!(0 <=fs_type && fs_type < FS_TYPE_TOTAL)) {
        return false;
    }


    for (int i = 0; i < 'z' - 'a'; i++) {
        if (g_device_filesystem[i].fs_type != FS_TYPE_NULL &&
            g_device_filesystem[i].device == device) {
            return false;
        }
    }

    if (g_device_filesystem[drive_letter - 'a'].fs_type != FS_TYPE_NULL) {
        g_device_filesystem[drive_letter - 'a'].fs_type = fs_type;
        g_device_filesystem[drive_letter - 'a'].device = device;
        g_device_filesystem[drive_letter - 'a'].fs_ctrl = g_operate_functions[fs_type].filesystem_new(device);
        g_device_filesystem[drive_letter - 'a'].opfuncs = &g_operate_functions[fs_type];
        return true;
    } else {
        return false;
    }
}

static inline  struct dev_fsctrl_s* drive_letter_to_ctrl(char letter)
{
    return &g_device_filesystem[tolower(letter) - 'a'];
}


/************************/

static inline bool path_check(const char* path)
{
    return isalpha(path[0]) && path[1] == ':';
}

static inline char path_drive_letter(const char* path)
{
    return path[0];
}

/* 去除盘符以外的部分 */
static inline const char* path_remain(const char* path)
{
    return path + 2;
}

static inline struct dev_fsctrl_s* path_to_ctrl(const char* path) {
    return drive_letter_to_ctrl(path_drive_letter(path));
}


#define FS_MAX_FILE_FD 2 >> 16

/* FIXME:似乎C语言全局数组默认就是NULL? 记不清楚了 */
fs_file_t* g_fs_files[FS_MAX_FILE_FD];

int fs_open(const char* path)
{
    if (!path_check(path)) {
        return FS_ERROR;
    }

    for (int fd = 0; fd < FS_MAX_FILE_FD; fd++) {
        if (g_fs_files[fd] == NULL) {
            struct dev_fsctrl_s* ctrl = path_to_ctrl(path);

            fs_file_t* file = ctrl->opfuncs->open(ctrl->device, ctrl->fs_ctrl, path_remain(path));
            if (file == NULL) {
                return FS_ERROR;
            }

            g_fs_files[fd] = file;
        }
    }
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



FS_DIR* fs_opendir(const char *path)
{
    return NULL;
}
int fs_readdir(FS_DIR* dir, char* name)
{
    return FS_ERROR;
}
void fs_closedir(FS_DIR* dir)
{
}
