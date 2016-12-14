#ifndef __UTILS_SYS__
#define __UTILS_SYS__

#include<stdio.h>
#include<stdbool.h>
size_t ft_filesize_from_fp(FILE* fp);
size_t ft_filesize(const char* path);

bool ft_create_bin_file(const char* path, size_t file_size);


#endif /* __UTILS_SYS__ */
