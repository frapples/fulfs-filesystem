#include "alloc.h"
#include<stdlib.h>

void* ft_malloc(size_t size)
{
    return malloc(size);
}

void* ft_malloc0(size_t size)
{
    return calloc((size_t)1, size);
}

void* ft_realloc(void* p, size_t size)
{
    return realloc(p, size);
}

void ft_free(void * p)
{
    free(p);
}
