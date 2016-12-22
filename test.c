#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<stdbool.h>
#include<string.h>

#include "device_io.h"
#include "datastruct/string.h"
#include "utils/sys.h"
#include "utils/testtools.h"
#include "utils/path.h"
#include "fulfs/filesystem.h"
#include "fulfs/superblock.h"
#include "fulfs/data_block.h"
#include "fulfs/inode.h"
#include "fulfs/base_file.h"
#include "fulfs/base_block_file.h"
#include "fs.h"
#include "utils/log.h"

#include <assert.h>

void bytearray_rand(char* arr, size_t size);
bool bytearray_equal(const char* a1, const char* a2, size_t size);
void bytearray_dump(const char* arr, size_t size);

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

    device_del(handle);
    return true;
}

bool test_inode_io(void)
{
    const char* path = "device_io_test.bin";
    int device = device_add(path);

    dev_inode_ctrl_t dev_inode_ctrl;
    dev_inode_ctrl_init(&dev_inode_ctrl, device, 4 * 1024, 1, 2048);

    /* 顺着写 倒着读 */
    inode_t inode[1000];
    inode_t inode2[1000];
    for (inode_no_t i = 0; i < 1000; i++) {
        bytearray_rand((char *)&inode[i], sizeof(inode_t));
        TEST_ASSERT(inode_dump(&dev_inode_ctrl, i, &inode[i]));
    }

    for (int i = 1000 - 1; i >= 0; i--) {
        TEST_ASSERT(inode_load(&dev_inode_ctrl, i, &inode2[i]));
    }

    for (inode_no_t i = 0; i < 1000; i++) {
        TEST_ASSERT(bytearray_equal((char *)&inode[i], (char *)&inode2[i], sizeof(inode_t)));
    }

    device_del(device);
    return true;
}

bool test_path(void)
{
    char path[FS_MAX_FILE_PATH];

    strcpy(path, "C:/abc/.././dfj");
    path_simplify(path);
    TEST_ASSERT(strcmp(path, "C:/dfj") == 0);

    strcpy(path, "C:/abc/def/../../dfj");
    path_simplify(path);
    TEST_ASSERT(strcmp(path, "C:/dfj") == 0);

    return true;
    const char* p = "fdfd hgjf fdie fd";
    size_t size = 0;
    while ((p = ft_string_split_next(p + size, " ", &size)) != NULL) {
        char buf[32];
        /* NOTE: strncpy的大坑 =_=*/
        strncpy(buf, p, size);
        buf[size] = '\0';
        printf("%s %d\n", buf, (int)size);
    }
    return true;
}

bool test_format(void)
{
    const char* path = "device_io_test.bin";
    size_t file_size = 32 * 1024 * 1024;
    if (ft_filesize(path) != file_size) {
        bool success = ft_create_bin_file(path, file_size);
        assert(success);
    }

    int device = device_add(path);
    TEST_ASSERT(fulfs_format(device, 4 * 1024 / 512));

    superblock_t sb;
    TEST_ASSERT(superblock_load(device, &sb));
    TEST_ASSERT(superblock_block_size(&sb) == 4 * 1024);
    /* TEST_ASSERT(superblock_used_size(&sb) == 0); */
    log_info("测试的文件系统data block范围:[%ld, %ld)\n", superblock_data_block_start(&sb),
             superblock_data_block_start(&sb) + superblock_data_block_size(&sb));


    dev_inode_ctrl_t dev_inode_ctrl;
    dev_inode_ctrl_init_from_superblock(&dev_inode_ctrl, device, &sb);
    inode_t inode;

    TEST_ASSERT(inode_load(&dev_inode_ctrl, 0, &inode));
    TEST_ASSERT(inode.mode == MODE_DIR);

    for (inode_no_t i = 1; i < INODE_MAX_COUNT; i++) {
        TEST_ASSERT(inode_load(&dev_inode_ctrl, i, &inode));
        TEST_ASSERT(inode.mode == 0);
    }


    base_file_t base_file;
    TEST_ASSERT(base_file_open(&base_file, device, &sb, 0));
    TEST_ASSERT(base_file_mode(&base_file) == MODE_DIR);

    device_del(device);
    return true;
}

bool test_base_block_file(void)
{
    const char* path = "device_io_test.bin";
    int device = device_add(path);

    superblock_t sb;
    TEST_ASSERT(superblock_load(device, &sb));

    inode_no_t inode_no;
    TEST_ASSERT(base_file_create(device, &sb, MODE_FILE, &inode_no));

    dev_inode_ctrl_t dev_inode_ctrl;
    dev_inode_ctrl_init_from_superblock(&dev_inode_ctrl, device, &sb);
    inode_t inode;
    TEST_ASSERT(inode_load(&dev_inode_ctrl, inode_no, &inode));

    for (int i = 0; i < 32; i++) {
        block_no_t block_1, block_2;
        TEST_ASSERT(base_block_file_push_block(device, &sb, &inode, &block_1));
        inode.size = (i + 1) * superblock_block_size(&sb);
        TEST_ASSERT(base_block_file_locate(device, &sb, &inode, i, &block_2));
        TEST_INT_EQUAL(block_1, block_2);
    }

    /* =_= 还是有错误 测试是否0号块被修改了 */
    TEST_ASSERT(superblock_load(device, &sb));
    TEST_ASSERT(sb.data_block_free_stack != 0);
    device_del(device);
    return true;
}

bool test_base_file(void)
{
    const char* path = "device_io_test.bin";
    int device = device_add(path);

    superblock_t sb;
    TEST_ASSERT(superblock_load(device, &sb));

    inode_no_t inode_no;
    TEST_ASSERT(base_file_create(device, &sb, MODE_FILE, &inode_no));
    TEST_ASSERT(inode_no > 0);

    base_file_t base_file;
    TEST_ASSERT(base_file_open(&base_file, device, &sb, inode_no));
    TEST_ASSERT(base_file_size(&base_file) == 0);
    TEST_ASSERT(base_file_tell(&base_file) == 0);

    int size = 1024 * 4 * 30;
    char buf[1024 * 4 * 30];
    char rand_buf[1024 * 4 * 30];

    TEST_ASSERT(base_file_read(&base_file, buf, size) == 0);

    bytearray_rand(rand_buf, size);
    int res = base_file_write(&base_file, rand_buf, size);
    TEST_ASSERT(res == size);
    TEST_ASSERT(base_file_size(&base_file) == (fsize_t)size);
    TEST_ASSERT(base_file_tell(&base_file) == (fsize_t)size);

    base_file_seek(&base_file, 0);
    res = base_file_read(&base_file, buf, size);
    TEST_ASSERT(res == size);
    TEST_ASSERT(bytearray_equal(buf, rand_buf, size));


    TEST_ASSERT(base_file_open(&base_file, device, &sb, 0));
    TEST_ASSERT(base_file_mode(&base_file) == MODE_DIR);

    device_del(device);
    return true;
}


bool test_fs(void)
{
    fs_init();

    const char* path = "device_io_test.bin";
    int device = device_add(path);

    TEST_ASSERT(fs_mount(device, 'A', FS_TYPE_FULFS));

    int fd = fs_open("A:/text.txt");
    TEST_ASSERT(fd != FS_ERROR);
    fs_close(fd);


    fd = fs_open("A:/text2.txt");
    TEST_ASSERT(fd != FS_ERROR);
    fs_close(fd);

    TEST_ASSERT(fs_mkdir("A:/testdir") == FS_SUCCESS);

    fd = fs_open("A:/text3.txt");
    TEST_ASSERT(fd != FS_ERROR);
    fs_close(fd);


    FS_DIR* dir = fs_opendir("A:/");
    char name[30];
    do {
        fs_readdir(dir, name);
        printf("%s\n", name);
    }while (name[0] != '\0');
    printf("\n");


    TEST_ASSERT(fs_rmdir("A:/testdir") == FS_SUCCESS);
    dir = fs_opendir("A:/");
    do {
        fs_readdir(dir, name);
        printf("%s\n", name);
    }while (name[0] != '\0');
    printf("\n");

    fs_closedir(dir);


    return true;
}


int main(int argc, char *argv[])
{
    srand(time(NULL));

    TestFunc funcs[] = {
        /* test_device_io, */
        /* test_inode_io, */
        test_path,
        test_format,
        /* test_base_block_file, */
        /* test_base_file, */
        test_fs,
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


void bytearray_dump(const char* arr, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        printf(" %3u ", (unsigned int)arr[i]);
        if ((i + 1) % 20 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}
