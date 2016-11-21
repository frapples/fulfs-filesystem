#ifndef __MEMORY_ALLOC__
#define __MEMORY_ALLOC__

#include<stddef.h>

/*
  这个模块提供了动态内存管理的功能。
  目前只是对标准库的包装而已。可是万一哪天我需要更好的内存分配器呢？
*/

#define FT_NEW(type_, size) ((type_ *)ft_malloc0(sizeof(type_) * size))

void* ft_malloc(size_t size);
void* ft_malloc0(size_t size);
void* ft_realloc(void* p, size_t size);
void ft_free(void * p);


#endif /* __MEMORY_ALLOC__ */
