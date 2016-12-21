#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "fs.h"

#include "memory/alloc.h"
#include "device_io.h"
#include "utils/sys.h"
#include "shell.h"

#define ERROR -1

int create(const char* path, size_t size);
int format(const char* path, const char* type, int block_size);

int enter(void);


int main(int argc, char* argv[])
{
    fs_init();

    int arg_count = argc - 1;
    char** args = argv + 1;

    if (arg_count == 0) {
        return enter();
    }

    const char* sub_cmd = args[0];
    if (strcmp(sub_cmd, "create") == 0) {

        if (arg_count < 3) {
            fprintf(stderr, "命令有误!\n");
            return ERROR;
        }

        char* end;
        size_t size = strtoumax(args[2], &end, 10);
        return create(args[1], size);

    }else if (strcmp(sub_cmd, "format") == 0) {

        if (arg_count < 4) {
            fprintf(stderr, "命令有误!\n");
            return ERROR;
        }

        char* end;
        int block_size = strtol(args[3], &end, 10);
        return format(args[1], args[2], block_size);

    } else if (strcmp(sub_cmd, "enter") == 0) {

        return enter();

    } else if (strcmp(sub_cmd, "help") == 0) {

        printf("命令帮助:\n"
               "create <文件路径> <文件大小>  创建容器文件\n"
               "format <容器路径> <文件系统类型> <扇区大小> \n"
               "enter  进入文件系统shell以操作文件系统\n"
               "help  查看此帮助 \n"
               );

    } else {
        fprintf(stderr, "命令有误!\n");
        return ERROR;
    }
}

int create(const char* path, size_t size)
{
    if (ft_create_bin_file(path, size)) {
        printf("创建容器文件%s成功，文件大小%d\n", path, (int)size);
        return ERROR;
    } else {
        printf("创建容器文件%s，大小%d失败。。。\n", path, (int)size);
        return 0;
    }
}

int format(const char* path, const char* type, int block_size)
{
    int fs_type;
    if (strcmp(type, "fulfs") == 0) {
        fs_type = FS_TYPE_FULFS;
    } else {
        printf("未知的文件系统类型。。。\n");
        return ERROR;
    }

    int device = device_add(path);
    if (!DEVICE_IO_SUCCESS(device)) {
        printf("设备挂载失败。。。\n");
        return ERROR;
    }

    int sectors_per_block = block_size / BYTES_PER_SECTOR;
    if (! (0 < sectors_per_block && sectors_per_block <= 16)) {
        printf("目前只支持512的1到16倍的块大小");
        device_del(device);
        return ERROR;
    }


    if (fs_format(device, sectors_per_block, fs_type) != FS_SUCCESS) {
        printf("格式化失败。。。\n");
        device_del(device);
        return ERROR;
    } else {
        printf("格式化成功! 块大小: %d\n", sectors_per_block * BYTES_PER_SECTOR);
        device_del(device);
        return 0;
    }
}

int enter(void)
{
    const char* config_file = "config.txt";

    FILE* fp = fopen(config_file, "rb");
    if (fp == NULL) {
        printf("打开配置文件失败！可能是没有配置文件？\n");
        return ERROR;
    }

    /* 挂载设备 */
    while (true) {
        char buf[32];
        char path[FS_MAX_FILE_PATH];
        int ret = fscanf(fp, "%s%s", buf, path);
        char drive_letter = buf[0];
        if (ret == EOF || ret == 0) {
            break;
        }

        if (ret != 2) {
            printf("解析配置文件%s错误！\n", config_file);
            fclose(fp);
            return ERROR;
        }

        int device = device_add(path);
        if (!DEVICE_IO_SUCCESS(device)) {
            printf("挂载失败：盘号%c 容器文件%s\n", drive_letter, path);
            continue;
        }

        if (!fs_mount(device, drive_letter, FS_TYPE_FULFS)) {
            printf("挂载失败：盘号%c 容器文件%s\n", drive_letter, path);
            continue;
        }

        printf("挂载成功：盘号%c 容器文件%s\n", drive_letter, path);
    }
    fclose(fp);



    printf("******** 欢迎使用fulfs文件系统shell！ ********\n");
    return shell_main();
}
