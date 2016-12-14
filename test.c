#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<stdbool.h>

#include "device_io.h"
#include "utils/sys.h"
#include "utils/testtools.h"
#include "fulfs/filesystem.h"

#include <assert.h>

void bytearray_rand(char* arr, size_t size);
bool bytearray_equal(const char* a1, const char* a2, size_t size);

bool test_device_io(void)
{

    const char* path = "device_io_test.bin";
    size_t file_size = 512 * 1024;
    if (ft_filesize(path) != file_size) {
        bool success = ft_create_bin_file(path, file_size);
        assert(success);
    }

    char buf[512 * 32] = {0};

    int handle = device_add(path);
    char rand_buf[512 * 32];
    bytearray_rand(rand_buf, 512 * 32);

    device_write(handle, 1, 1, rand_buf);
    device_read(handle, 1, 1, buf);

    TEST_ASSERT(bytearray_equal(buf, rand_buf, 512));


    device_write(handle, 120, 32, rand_buf);
    device_read(handle, 120, 32, buf);

    TEST_ASSERT(bytearray_equal(buf, rand_buf, 512));
    return true;
}

bool test_format(void)
{
    const char* path = "device_io_test.bin";
    size_t file_size = 512 * 1024;
    if (ft_filesize(path) != file_size) {
        bool success = ft_create_bin_file(path, file_size);
        assert(success);
    }

    int handle = device_add(path);
    fulfs_errcode_t errcode = fulfs_format(handle, 4 * 1024 / 512);
    TEST_ASSERT(errcode == FULFS_SUCCESS);

    return true;
}

bool test_base_file(void)
{
    return true;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    TestFunc funcs[] = {
        test_device_io,
        test_format,
        test_base_file
    };
    return test_main(funcs, sizeof(funcs) / sizeof(*funcs));
}


bool bytearray_equal(const char* a1, const char* a2, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (a1[i] != a2[i]) {
            return false;
        }
    }
    return true;
}

void bytearray_rand(char* arr, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        arr[i] = rand() % 64;
    }
}
