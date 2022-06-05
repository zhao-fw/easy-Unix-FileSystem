#ifndef DEVICE_H
#define DEVICE_H

#include"File.h"
#include <cstdio>
#include <cstdlib>

class Device {
private:

    /* �����ļ������linux��ʹ�� 
     * int fd;
    */

    /* �����ļ�ָ�� linux��Windows������ʹ�� */
    FILE* fp;

    /* �����ļ� */
    const char* deviceFile = "myDisk.img";

public:
    Device();
    ~Device();

    /* ��龵���ļ��Ƿ���� */
    bool Exist();

    /* �򿪾����ļ� */
    void NewD();

    /* ʵ��д���̺��� */
    void WriteD(const void* buffer, unsigned int size, int offset = -1);

    /* ʵ��д���̺��� */
    void ReadD(void* buffer, unsigned int size, int offset = -1);
};

#endif