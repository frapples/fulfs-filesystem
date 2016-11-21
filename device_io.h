#ifndef __DEVICE_IO__
#define __DEVICE_IO__

#include<stddef.h>

/*
  此模块是模拟对磁盘进行以扇区为单位的读写。

  对磁盘上的所有扇区，会对其从0编号。每个扇区占512字节。
 */

#define DEVICE_IO_ERROR -1

/* 将外部系统的path文件模拟成系统的一个磁盘，返回其句柄，错误则返回-1 */
int device_add(const char* path);

/* 把设备卸载 */
void device_del(int handle);


/* 从section_no扇区开始，读取count个扇区的内容到buf中。
   返回读取的扇区数。错误返回-1
 */
int device_read(int handle, int section_no, int count, char* buf);

/* 将buf中内容写入到从section_no扇区开始，count个扇区中
   返回写入的扇区数。错误返回-1
 */
int device_write(int handle, int section_no, int count, const char* buf);

int device_section_count(int handle);

#endif /* __DEVICE_IO__ */
