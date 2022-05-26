#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include"Buf.h"
#include"Device.h"


class BufferManager {
public:
    static const int NBUF = 100;			/* 缓存控制块、缓冲区的数量 */
    static const int BUFFER_SIZE = 512;		/* 缓冲区大小。 以字节为单位 */

private:
    Buf* bFreeList;							    /* 缓存队列控制块 */
    Buf m_Buf[NBUF];						    /* 缓存控制块数组 */
    unsigned char Buffer[NBUF][BUFFER_SIZE];	/* 缓冲区数组 */
    Device* m_device;

public:
    BufferManager();
    ~BufferManager();

    Buf* GetBlk(int blkno);         /* 申请一块缓存，用于读写文件中的字符块blkno。*/
    void RelseB(Buf* bp);           /* 释放缓存控制块buf */

    Buf* ReadB(int blkno);          /* 读一个磁盘块，blkno为目标磁盘块逻辑块号。 */
    void WriteB(Buf* bp);           /* 写一个磁盘块 */
    void DWriteB(Buf* bp);          /* 延迟写磁盘块 */

    void ClearB(Buf* bp);           /* 清空缓冲区内容 */
    void FlushB();                  /* 将队列中延迟写的缓存全部输出到磁盘 */
    void FormatBuf();               /* 格式化所有Buffer */

private:
    void InitList();
    void debug();
};

#endif