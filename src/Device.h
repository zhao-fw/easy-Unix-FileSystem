#ifndef DEVICE_H
#define DEVICE_H

#include"File.h"
#include <cstdio>
#include <cstdlib>

class Device {
private:

    /* 磁盘文件句柄，linux下使用 
     * int fd;
    */

    /* 磁盘文件指针 linux和Windows都可以使用 */
    FILE* fp;

    /* 磁盘文件 */
    const char* deviceFile = "myDisk.img";

public:
    Device();
    ~Device();

    /* 检查镜像文件是否存在 */
    bool Exist();

    /* 打开镜像文件 */
    void NewD();

    /* 实际写磁盘函数 */
    void WriteD(const void* buffer, unsigned int size, int offset = -1);

    /* 实际写磁盘函数 */
    void ReadD(void* buffer, unsigned int size, int offset = -1);
};

#endif