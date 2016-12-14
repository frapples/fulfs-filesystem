#ifndef __TESTTOOLS__H__
#define __TESTTOOLS__H__

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

#define TEST_ASSERT(exp)                                                \
    do {                                                                \
        if (!(exp)) {                                                   \
            fprintf(stderr, "Test error in %s:%d: %s\n", __FILE__, __LINE__, #exp); \
            return false;                                               \
        }                                                               \
    } while(0)                                                          \

typedef bool (*TestFunc)(void);

int test_main(TestFunc funcs[], size_t size);

#endif /* __TESTTOOLS__H__ */
