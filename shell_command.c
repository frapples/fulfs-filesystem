#include "shell_command.h"

int cmd_pwd(int argc, char* argv[])
{
    char path[FS_MAX_FILE_PATH];
    fs_getcwd(path, FS_MAX_FILE_PATH);
    printf("%s\n", path);
    return 0;
}

int cmd_cd(int argc, char* argv[])
{
    printf("%d\n", argc);

    for (int i = 0; i < argc; i++) {
        printf("[%s]\n", argv[i]);
    }
    return 0;
}

int cmd_ls(int argc, char* argv[])
{
    return 0;
}

int cmd_mkdir(int argc, char* argv[])
{
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
