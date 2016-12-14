#include "log.h"

#include <stdlib.h>
#include <stdio.h>

static const char* level_to_str(int level)
{
    static const char* strs[] = {"DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};
    return strs[level];
}

static FILE* log_file_fp = NULL;

void log_set_file(FILE* fp)
{
    log_file_fp = fp;
}

FILE* log_get_file(void)
{
    if (log_file_fp == NULL) {
        return stderr;
    } else {
        return log_file_fp;
    }
}


void log_log_valist(int level, const char* format, va_list ap)
{
    FILE* fp = log_get_file();
    fprintf(fp, "%s:", level_to_str(level));
    vfprintf(fp, format, ap);
    fprintf(fp, "\n");
}

