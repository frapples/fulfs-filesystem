#include "shell.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "memory/alloc.h"
#include "datastruct/string.h"
#include "fs.h"

#include "shell_command.h"

#define MAX_LINE (1024 * 4)
#define MAX_ARGC 1024

static int cmd_dispath(const char* cmd, int argc, char* argv[]);

int shell_main(void)
{
    fs_chdir("A:/");
    char line[MAX_LINE];

    char* argv[MAX_ARGC];
    while (true) {
        char cd[FS_MAX_FILE_PATH];
        fs_getcwd(cd, FS_MAX_FILE_PATH);
        printf("fulfs@%s", cd);
        printf(" >>> ");
        if (fgets(line, sizeof(line) / sizeof(*line), stdin) == NULL) {
            return 0;
        }

        /* 解析参数 */
        int argc = 0;
        const char* p;
        size_t size = 0;
        const char* next = line;
        while (((p = ft_string_split_next(next, " ", &size)) != NULL) && argc <= MAX_ARGC) {
            argv[argc] = (char* )p;
            ft_str_strip(argv[argc]);

            if (p[size] == '\0') {
                next = p + size;
            } else {
                next = p + size + 1;
            }

            ((char *)p)[size] = '\0';

            if (argv[argc][0] != '\0') {
                argc++;
            }

        }

        if (argc > 0) {
            cmd_dispath(argv[0], argc - 1, argv + 1);
        }
    }

    return 0;
}

int cmd_dispath(const char* cmd, int argc, char* argv[])
{
    struct {
        const char* name;
        shell_cmd_f* func;
    }commands[] = {
        {"pwd", cmd_pwd},
        {"cd", cmd_cd},
        {"rmdir", cmd_rmdir},
        {"mkdir", cmd_mkdir},
        {"ls", cmd_ls},
        {"cp", cmd_cp},
        {"mv", cmd_mv},
        {"rm", cmd_rm},
        {"tree", cmd_tree},
        {"ln", cmd_ln},
        {"stat", cmd_stat},
        {"cat", cmd_cat},
        {"df", cmd_df},
        {"createfile", cmd_createfile},
    };

    for (size_t i = 0; i < sizeof(commands) / sizeof(*commands); i++) {
        if (strcmp(cmd, commands[i].name) == 0) {
            return commands[i].func(argc, argv);
        }
    }

    printf("不存在命令%s！\n", cmd);
    return -1;
}
