#include "shell.h"

#include <stdio.h>
#include <stdbool.h>

#include "fs.h"

#define MAX_LINE (1024 * 4)
int shell_main(void)
{
    char line[MAX_LINE];

    while (true) {
        printf(">>> ");
        if (fgets(line, sizeof(line) / sizeof(*line), stdin) == NULL) {
            return 0;
        }
        printf("%s", line);
    }

    return 0;
}
