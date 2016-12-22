#include "device_io.h"
#include<stdio.h>
#include<string.h>

#include"memory/alloc.h"
#include"utils/sys.h"
#include"utils/log.h"

struct _device_s{
    char* path;
    FILE* fp;
    sector_no_t section_count;
};

#define MAX_DEVICE_COUNT 1024
struct _device_s* device_handle[MAX_DEVICE_COUNT] = {NULL};

int device_add(const char* path)
{
    /* 已经挂载的文件就不允许别人挂载了 */
    for (int i = 0; i < MAX_DEVICE_COUNT; i++) {
        if (device_handle[i] != NULL && strcmp(device_handle[i]->path, path) == 0) {
            log_info("%s文件已经被当成设备挂载了，请先卸载", path);
            return DEVICE_IO_ERROR;
        }
    }

    for (int i = 0; i < MAX_DEVICE_COUNT; i++) {
        if (device_handle[i] == NULL) {
            device_handle[i] = FT_NEW(struct _device_s, 1);

            device_handle[i]->path = FT_NEW(char, strlen(path) + 1);
            strcpy(device_handle[i]->path, path);

            device_handle[i]->fp = fopen(path, "r+b");
            device_handle[i]->section_count = ft_filesize_from_fp(device_handle[i]->fp) /  BYTES_PER_SECTOR;

            return i;
        }
    }

    return DEVICE_IO_ERROR;
}


void device_del(device_handle_t handle)
{
    if (handle < MAX_DEVICE_COUNT && handle >= 0 && device_handle[handle] != NULL) {
        fclose(device_handle[handle]->fp);
        ft_free(device_handle[handle]->path);
        ft_free(device_handle[handle]);
        device_handle[handle] = NULL;
    }
}

static struct _device_s* handle_to_struct(device_handle_t handle)
{
    if (!(handle < MAX_DEVICE_COUNT && handle >= 0 && device_handle[handle] != NULL)) {
        return NULL;
    } else {
        return device_handle[handle];
    }
}

int device_read(device_handle_t handle, sector_no_t sector_no, int count, char* buf)
{
    struct _device_s* device = handle_to_struct(handle);
    if (device == NULL) {
        return DEVICE_IO_ERROR;
    }

    size_t offset = sector_no * BYTES_PER_SECTOR;

    if (!(sector_no + count <= device->section_count)) {
        count = device->section_count - sector_no;
    }

    if(fseek(device->fp, offset, SEEK_SET) != 0) {
        return DEVICE_IO_ERROR;
    }

    fread(buf, BYTES_PER_SECTOR, count, device->fp);

    if (ferror(device->fp)) {
        return DEVICE_IO_ERROR;
    } else {
        return count;
    }
}


int device_write(device_handle_t handle, sector_no_t sector_no, int count, const char* buf)
{
    struct _device_s* device = handle_to_struct(handle);
    if (device == NULL) {
        return DEVICE_IO_ERROR;
    }


    size_t offset = sector_no * BYTES_PER_SECTOR;

    if (!(sector_no + count <= device->section_count)) {
        count = device->section_count - sector_no;
    }

    if(fseek(device->fp, offset, SEEK_SET) != 0) {
        return DEVICE_IO_ERROR;
    }

    fwrite(buf, BYTES_PER_SECTOR, count, device->fp);


    if (ferror(device->fp)) {
        return DEVICE_IO_ERROR;
    } else {
        return count;
    }
}

int device_section_count(device_handle_t handle)
{
    struct _device_s* device = handle_to_struct(handle);
    if (device == NULL) {
        return 0;
    } else {
        return device->section_count;
    }
}
