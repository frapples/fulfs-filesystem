#ifndef __UTILS_MATH__H__
#define __UTILS_MATH__H__



/* 每组有per_group个，n个可以分成几组？ */
static inline unsigned long count_groups(unsigned long n, unsigned long per_group)
{
    return n / per_group + ((n % per_group == 0) ? 0 : 1);
}

static inline int min_int(int a, int b)
{
    return a < b ? a : b;
}


#endif /* __UTILS_MATH__H__ */
