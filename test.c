#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<stdbool.h>

#include "device_io.h"
#include "utils/sys.h"

void bytearray_rand(char* arr, size_t size);
bool bytearray_equal(const char* a1, const char* a2, size_t size);

int test_device_io(void)
{

    FILE* fp = fopen("device_io_test.bin", "r+b");
    char buf[512 * 32] = {0};
    for (int i = 0; i < 1024; i++) {
        fwrite(buf, 512, 1, fp);
    }
    fclose(fp);

    int handle = device_add("device_io_test.bin");
    char rand_buf[512 * 32];
    bytearray_rand(rand_buf, 512 * 32);

    device_write(handle, 1, 1, rand_buf);
    device_read(handle, 1, 1, buf);
    if (bytearray_equal(buf, rand_buf, 512)) {
        return 0;
    } else {
        return -1;
    }


    device_write(handle, 120, 32, rand_buf);
    device_read(handle, 120, 32, buf);
    if (bytearray_equal(buf, rand_buf, 512)) {
        return 0;
    } else {
        return -1;
    }

}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    return test_device_io();
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
