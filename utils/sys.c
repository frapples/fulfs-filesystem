#include "sys.h"

#include<stdio.h>

size_t ft_filesize_from_fp(FILE* fp)
{
    size_t old = ftell(fp);
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, old, SEEK_SET);
    return size;
}

size_t ft_filesize(const char* path)
{
    FILE* fp = fopen(path, "rb");
    if (fp == NULL) {
        return 0;
    }

    size_t size = ft_filesize_from_fp(fp);
    fclose(fp);
    return size;
}

bool ft_create_bin_file(const char* path, size_t file_size)
{
    FILE* fp = fopen(path, "wb");
    if (fp == NULL) {
        return false;
    }

    int buf_size = 512;
    char buf[512] = {0};

    for (size_t i = 0; i < file_size; i += buf_size) {
        size_t writed_size;
        if (i + buf_size > file_size) {
            writed_size = file_size - i;
        } else {
            writed_size = buf_size;
        }
        size_t success = fwrite(buf, writed_size, 1, fp);
        if (success < 1) {
            fclose(fp);
            remove(path);
            return false;
        }
    }
    fclose(fp);
    return true;
}

char ft_human_size(size_t size, size_t* p_size)
{
    if (size > 1024) {
        size /= 1024;
    } else {
        *p_size = size;
        return 'B';
    }

    if (size > 1024) {
        size /= 1024;
    } else {
        *p_size = size;
        return 'K';
    }

    if (size > 1024) {
        size /= 1024;
    } else {
        *p_size = size;
        return 'M';
    }

    if (size > 1024) {
        size /= 1024;
    } else {
        *p_size = size;
        return 'G';
    }

    *p_size = size;
    return 'T';
}
