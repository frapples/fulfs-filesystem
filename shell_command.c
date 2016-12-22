#include "shell_command.h"

#include <string.h>
#include "utils/path.h"
int cmd_pwd(int argc, char* argv[])
{
    char path[FS_MAX_FILE_PATH];
    fs_getcwd(path, FS_MAX_FILE_PATH);
    printf("%s\n", path);
    return 0;
}

int cmd_cd(int argc, char* argv[])
{
    if (argc <= 0) {
        printf("命令有误\n");
        return -1;
    }

    if (fs_chdir(argv[0]) == FS_SUCCESS) {
        return 0;
    } else {
        printf("改变失败，可能目录不存在\n");
        return -1;
    }

}

int cmd_ls(int argc, char* argv[])
{
    const char* path;
    char wd[FS_MAX_FILE_PATH];
    if (argc <= 0) {
        fs_getcwd(wd, FS_MAX_FILE_PATH);
        path = wd;
    } else {
        path = argv[0];
    }

    FS_DIR* dir = fs_opendir(path);
    if (dir == NULL) {
        printf("列出目录信息失败\n");
    }

    char name[32];
    do {
        fs_readdir(dir, name);
        printf("%s ", name);
    } while(name[0] != '\0');
    printf("\n");

    fs_closedir(dir);

    return 0;
}

int cmd_mkdir(int argc, char* argv[])
{
    if (argc <= 0) {
        printf("命令有误\n");
        return -1;
    }

    if (fs_mkdir(argv[0]) == FS_SUCCESS) {
        return 0;
    } else {
        printf("建立失败\n");
        return -1;
    }
}

static bool tree(char* path, int* p_level)
{
    for (int i = 0; i < *p_level; i++) {
            printf("   ");
    }
        printf("%s\n", path_p_basename(path));

    FS_DIR* dir = fs_opendir(path);
    char name[32];

    while ((fs_readdir(dir, name), name[0] != '\0')) {
        size_t tail = strlen(path);
        path_join(path, FS_MAX_FILE_PATH, name);

        (*p_level)++;
        tree(path, p_level);
        (*p_level)--;
        path[tail] = '\0';
    }
    fs_closedir(dir);
    return true;
}

int cmd_tree(int argc, char* argv[])
{
    char path[FS_MAX_FILE_PATH];
    if (argc <= 0) {
        fs_getcwd(path, FS_MAX_FILE_PATH);
    } else {
        strcpy(path, argv[0]);
    }

    int level = 0;
    tree(path, &level);
    return 0;
}

int cmd_rmdir(int argc, char* argv[])
{
    return 0;
}

int cmd_cp(int argc, char* argv[])
{
    return 0;
}

int cmd_mv(int argc, char* argv[])
{
    return 0;
}

int cmd_rm(int argc, char* argv[])
{
    return 0;
}
