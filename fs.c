#include "fs.h"

#include "fs_def.h"
#include "fulfs/fulfs.h"
#include "memory/alloc.h"
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
struct {
    fs_file_t* file;
    char drive_letter;
} g_fs_files[FS_MAX_FILE_FD];

int fs_open(const char* path)
{
    if (!path_check(path)) {
        return FS_ERROR;
    }

    for (int fd = 0; fd < FS_MAX_FILE_FD; fd++) {
        if (g_fs_files[fd].file == NULL) {
            struct dev_fsctrl_s* ctrl = path_to_ctrl(path);

            fs_file_t* file = ctrl->opfuncs->open(ctrl->device, ctrl->fs_ctrl, path_remain(path));
            if (file == NULL) {
                return FS_ERROR;
            }

            g_fs_files[fd].file = file;
            g_fs_files[fd].drive_letter = path_drive_letter(path);
        }
    }
    return FS_ERROR;
}

void fs_close(int fd)
{
    if (fd < FS_MAX_FILE_FD && g_fs_files[fd].file != NULL) {
        struct dev_fsctrl_s* ctrl = drive_letter_to_ctrl(g_fs_files[fd].drive_letter);
        ctrl->opfuncs->close(g_fs_files[fd].file);
    }

}

int fs_read(int fd, char* buf, int count)
{
    if (fd < FS_MAX_FILE_FD && g_fs_files[fd].file != NULL) {
    struct dev_fsctrl_s* ctrl = drive_letter_to_ctrl(g_fs_files[fd].drive_letter);
    return ctrl->opfuncs->read(g_fs_files[fd].file, buf, count);
    } else {
        return FS_ERROR;
    }
}
int fs_write(int fd, const char* buf, int count)
{
    if (fd < FS_MAX_FILE_FD && g_fs_files[fd].file != NULL) {
        struct dev_fsctrl_s* ctrl = drive_letter_to_ctrl(g_fs_files[fd].drive_letter);
        return ctrl->opfuncs->write(g_fs_files[fd].file, buf, count);
    } else {
        return FS_ERROR;
    }
}

bool fs_ftruncate(int fd, fs_off_t size)
{
    if (fd < FS_MAX_FILE_FD && g_fs_files[fd].file != NULL) {
        struct dev_fsctrl_s* ctrl = drive_letter_to_ctrl(g_fs_files[fd].drive_letter);
        return ctrl->opfuncs->ftruncate(g_fs_files[fd].file, size);
    } else {
        return FS_ERROR;
    }
}

fs_off_t fs_lseek(int fd, fs_off_t off, int where)
{
    if (fd < FS_MAX_FILE_FD && g_fs_files[fd].file != NULL) {
        struct dev_fsctrl_s* ctrl = drive_letter_to_ctrl(g_fs_files[fd].drive_letter);
        return ctrl->opfuncs->lseek(g_fs_files[fd].file, off, where);
    } else {
        return FS_ERROR;
    }
}


int fs_mkdir(const char* path)
{
    if (!path_check(path)) {
        return FS_ERROR;
    }

    struct dev_fsctrl_s* ctrl = path_to_ctrl(path);
    if (ctrl->opfuncs->mkdir(ctrl->device, ctrl->fs_ctrl, path)) {
        return FS_SUCCESS;
    } else {
        return FS_ERROR;
    }
}

int fs_rmdir(const char* path)
{
    if (!path_check(path)) {
        return FS_ERROR;
    }

    struct dev_fsctrl_s* ctrl = path_to_ctrl(path);
    if (ctrl->opfuncs->rmdir(ctrl->device, ctrl->fs_ctrl, path)) {
        return FS_SUCCESS;
    } else {
        return FS_ERROR;
    }
}

int fs_link(const char* src_path, const char* new_path)
{
    if (!path_check(src_path) || !path_check(new_path)) {
        return FS_ERROR;
    }

    if ((path_drive_letter(src_path)) != path_drive_letter(new_path)) {
        return FS_ERROR;
    }

    struct dev_fsctrl_s* ctrl = path_to_ctrl(src_path);
    if (ctrl->opfuncs->link(ctrl->device, ctrl->fs_ctrl, src_path, new_path)) {
        return FS_SUCCESS;
    } else {
        return FS_ERROR;
    }
}
int fs_unlink(const char* path)
{
    if (!path_check(path)) {
        return FS_ERROR;
    }
    struct dev_fsctrl_s* ctrl = path_to_ctrl(path);
    if (ctrl->opfuncs->unlink(ctrl->device, ctrl->fs_ctrl, path)) {
        return FS_SUCCESS;
    } else {
        return FS_ERROR;
    }
}

int fs_symlink(const char* src_path, const char* new_path)
{
    if (!path_check(src_path) || !path_check(new_path)) {
        return FS_ERROR;
    }

    if ((path_drive_letter(src_path)) != path_drive_letter(new_path)) {
        return FS_ERROR;
    }

    struct dev_fsctrl_s* ctrl = path_to_ctrl(src_path);
    if (ctrl->opfuncs->symlink(ctrl->device, ctrl->fs_ctrl, src_path, new_path)) {
        return FS_SUCCESS;
    } else {
        return FS_ERROR;
    }
}
int fs_readlink(const char *path, char *buf, size_t size)
{
    if (!path_check(path)) {
        return FS_ERROR;
    }
    struct dev_fsctrl_s* ctrl = path_to_ctrl(path);
    if (ctrl->opfuncs->readlink(ctrl->device, ctrl->fs_ctrl, path, buf, size)) {
        return FS_SUCCESS;
    } else {
        return FS_ERROR;
    }
    return FS_ERROR;
}


int fs_stat(const char *path, struct fs_stat *buf)
{
    if (!path_check(path)) {
        return FS_ERROR;
    }
    struct dev_fsctrl_s* ctrl = path_to_ctrl(path);
    if (ctrl->opfuncs->stat(ctrl->device, ctrl->fs_ctrl, path, buf)) {
        return FS_SUCCESS;
    } else {
        return FS_ERROR;
    }
}


struct _fs_dir_wrapper_s
{
    fs_dir_t* dir;
    char drive_letter;
};

FS_DIR* fs_opendir(const char *path)
{
    if (!path_check(path)) {
        return NULL;
    }


    struct dev_fsctrl_s* ctrl = path_to_ctrl(path);
    fs_dir_t* dir = ctrl->opfuncs->opendir(ctrl->device, ctrl->fs_ctrl, path);
    if (dir == NULL) {
        return NULL;
    }

    FS_DIR* dir_wrapper = FT_NEW(FS_DIR, 1);
    dir_wrapper->dir = dir;
    dir_wrapper->drive_letter = path_drive_letter(path);
    return dir_wrapper;
}

int fs_readdir(FS_DIR* dir, char* name)
{
    if (dir != NULL) {
        struct dev_fsctrl_s* ctrl = drive_letter_to_ctrl(dir->drive_letter);
        if (ctrl->opfuncs->readdir(dir->dir, name)) {
            return FS_SUCCESS;
        } else {
            return FS_ERROR;
        }
    } else {
        return FS_ERROR;
    }
}

void fs_closedir(FS_DIR* dir)
{
    if (dir != NULL) {
        struct dev_fsctrl_s* ctrl = drive_letter_to_ctrl(dir->drive_letter);
        ctrl->opfuncs->closedir(dir->dir);
        ft_free(dir);
    }
}
