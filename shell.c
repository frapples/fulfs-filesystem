#include "shell.h"

#include <stdio.h>
#include <stdbool.h>

#include "fs.h"

#define MAX_LINE (1024 * 4)
int shell_main(void)
{
    fs_chdir("A:/");
    char line[MAX_LINE];

    while (true) {
        char cd[FS_MAX_FILE_PATH];
        fs_getcwd(cd, FS_MAX_FILE_PATH);
        printf("%s", cd);
        printf(" >>> ");
        if (fgets(line, sizeof(line) / sizeof(*line), stdin) == NULL) {
            return 0;
        }
        printf("%s", line);
    }

    return 0;
}
