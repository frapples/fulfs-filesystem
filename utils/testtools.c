#include "testtools.h"

#include <stdio.h>

int test_main(TestFunc funcs[], size_t size)
{
    for (size_t i = 0; i < size; i++) {
        bool result =  (*funcs[i])();
        if (!result) {
            return i + 1;
        }
    }
    return 0;
}
