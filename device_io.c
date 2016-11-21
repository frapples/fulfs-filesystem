#include "device_io.h"
#include<stdio.h>

#include"memory/alloc.h"
#include"utils/sys.h"

struct _device_s{
    FILE* fp;
    size_t section_count;
};

#define MAX_DEVICE_COUNT 1024
struct _device_s* device_handle[MAX_DEVICE_COUNT] = {NULL};

int device_add(const char* path)
{
    for (int i = 0; i < MAX_DEVICE_COUNT; i++) {
        if (device_handle[i] == NULL) {
            device_handle[i] = FT_NEW(struct _device_s, 1);

            device_handle[i]->fp = fopen(path, "wb");
            device_handle[i]->section_count = ft_filesize(device_handle[i]->fp) / 512;

            return i;
        }
    }

    return DEVICE_IO_ERROR;
}


void device_del(int handle)
{
    if (handle < MAX_DEVICE_COUNT && device_handle[handle] != NULL) {
        ft_free(device_handle[handle]);
        device_handle[handle] = NULL;
    }
}

static struct _device_s* handle_to_struct(int handle)
{
    if (!(handle < MAX_DEVICE_COUNT && device_handle[handle] != NULL)) {
        return NULL;
    } else {
        return device_handle[handle];
    }
}

size_t device_read(int handle, size_t section_no, size_t count, char* buf)
{
    struct _device_s* device = handle_to_struct(handle);
    if (device == NULL) {
        return -1;
    }

    size_t offset = section_no * 512;

    if (!(section_no + count <= device->section_count)) {
        count = device->section_count - section_no;
    }

    fseek(device->fp, offset, SEEK_SET);
    fread(buf, 512, count, device->fp);

    if (ferror(device->fp)) {
        return -1;
    } else {
        return count;
    }
}


size_t decice_write(int handle, size_t section_no, size_t count, char* buf)
{
    struct _device_s* device = handle_to_struct(handle);
    if (device == NULL) {
        return -1;
    }


    size_t offset = section_no * 512;

    if (!(section_no + count <= device->section_count)) {
        count = device->section_count - section_no;
    }

    fseek(device->fp, offset, SEEK_SET);

    fwrite(buf, 512, count, device->fp);


    if (ferror(device->fp)) {
        return -1;
    } else {
        return count;
    }
}

