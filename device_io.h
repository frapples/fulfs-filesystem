#ifndef __DEVICE_IO__
#define __DEVICE_IO__

#include<stddef.h>
#include<stdint.h>

/*
  此模块是模拟对磁盘进行以扇区为单位的读写。

  对磁盘上的所有扇区，会对其从0编号。每个扇区占512字节。
 */

/* 扇区大小 */
#define BYTES_PER_SECTOR 512

#define DEVICE_IO_ERROR -1

#define DEVICE_IO_SUCCESS(handle) (handle >= 0)

typedef int device_handle_t;
typedef uint64_t sector_no_t;

/* 将外部系统的path文件模拟成系统的一个磁盘，返回其句柄，错误则返回-1 */
device_handle_t device_add(const char* path);

/* 把设备卸载 */
void device_del(device_handle_t handle);


/* 从section_no扇区开始，读取count个扇区的内容到buf中。
   返回读取的扇区数。错误返回-1
 */
int device_read(device_handle_t handle, sector_no_t sector_no, int count, char* buf);

/* 将buf中内容写入到从section_no扇区开始，count个扇区中
   返回写入的扇区数。错误返回-1
 */
int device_write(device_handle_t handle, sector_no_t sector_no, int count, const char* buf);

int device_section_count(device_handle_t handle);

#endif /* __DEVICE_IO__ */
