#ifndef __UTILS__PATH__H__
#define __UTILS__PATH__H__

#include <stddef.h>

void path_simplify(char* path);

void path_dirname(const char* path, char* dir);
void path_basename(const char* path, char* name, size_t size);

const char* path_p_basename(const char* path);

char* path_join(char* path1, size_t size, const char* path);

#endif /* __UTILS__PATH__H__ */
