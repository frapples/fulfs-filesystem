#include "device_io.h"
#include<stdio.h>

#include"memory/alloc.h"
#include"utils/sys.h"

struct _device_s{
    FILE* fp;
    sector_no_t section_count;
};

#define MAX_DEVICE_COUNT 1024
struct _device_s* device_handle[MAX_DEVICE_COUNT] = {NULL};

int device_add(const char* path)
{
    for (int i = 0; i < MAX_DEVICE_COUNT; i++) {
        if (device_handle[i] == NULL) {
            device_handle[i] = FT_NEW(struct _device_s, 1);

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
