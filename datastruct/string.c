#include "string.h"

#include <string.h>

const char* ft_string_split_next(const char* str, const char* split, size_t* p_size)
{

    const char* start;


    size_t split_size = strlen(split);
    const char *pstr = str;
    const char* p = strstr(str, split);
    while (p == pstr) {
        pstr += split_size;
        p = strstr(pstr, split);
    }
    if (str[0] == '\0') {
        return NULL;
    }

    start = pstr;
    if (p == NULL) {
        *p_size = strlen(pstr);
    } else {
        *p_size = p - start;
    }
    return start;
}

void ft_str_strip(char* str)
{
    long start = 0;
    while (strchr(" \n\t", str[start]) != NULL) {
        start++;
    }


    size_t size = strlen(str);
    if (size == 0) {
        return;
    }

    if (size == 1 && strchr(" \n\t", str[0])) {
        str[0] = '\0';
        return;
    }

    long end = size - 1;
    while (strchr(" \n\t", str[end]) != NULL) {
        end--;
    }

    if (start <= end) {
        for (long i = 0; i <= end - start; i++) {
            str[i] = str[i + start];
        }
        str[end + 1] = '\0';
    }
}

void ft_str_reverse(char* str)
{
    size_t size = strlen(str);
    for (size_t i = 0; i < size / 2; i++) {
        char tmp = str[i];
        str[i] = str[size - 1 - i];
        str[size - 1 - i] = tmp;
    }
}
