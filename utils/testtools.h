#ifndef __TESTTOOLS__H__
#define __TESTTOOLS__H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define TEST_ASSERT(exp)                                                \
    do {                                                                \
        if (!(exp)) {                                                   \
            fprintf(stderr, "Test error in %s:%d: %s\n", __FILE__, __LINE__, #exp); \
            return false;                                               \
        }                                                               \
    } while(0)                                                          \

#define TEST_ASSERT_MSG(exp, msg_exp)                                   \
    do {                                                                \
        if (!(exp)) {                                                   \
            fprintf(stderr, "Test error in %s:%d: %s\n", __FILE__, __LINE__, #exp); \
            msg_exp;                                                    \
            return false;                                               \
        }                                                               \
    } while(0)                                                          \

#define TEST_INT_EQUAL(a, b)                                            \
    do {                                                                \
        int64_t __a = a;                                                \
        int64_t __b = b;                                                \
                                                                        \
        if (!(__a == __b)) {                                            \
            fprintf(stderr, "Test error in %s:%d： %s == %s. In fact：%lld != %lld\n", __FILE__, __LINE__, #a, #b, __a, __b); \
            return false;                                               \
        }                                                               \
                                                                        \
    }while(0)                                                           \

typedef bool (*TestFunc)(void);

int test_main(TestFunc funcs[], size_t size);

#endif /* __TESTTOOLS__H__ */
