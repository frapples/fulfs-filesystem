#ifndef __DATASRUCT_STRING__H__
#define __DATASRUCT_STRING__H__

#include <stddef.h>
#include <stdbool.h>


const char* ft_string_split_next(const char* str, const char* split, size_t* p_size);

void ft_str_strip(char* str);


void ft_str_reverse(char* str);

#endif /* __DATASRUCT_STRING__H__ */
