#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include"Buf.h"
#include"Device.h"


class BufferManager {
public:
    static const int NBUF = 100;			/* 缓存控制块、缓冲区的数量 */
    static const int BUFFER_SIZE = 512;		/* 缓冲区大小。 以字节为单位 */

private:
    Buf* bufferList;							/* 缓存队列控制块 */
    Buf nBuffer[NBUF];						/* 缓存控制块数组 */
    unsigned char Buffer[NBUF][BUFFER_SIZE];	/* 缓冲区数组 */
    unordered_map<int, Buf*> map;
    Device* device;

public:
    BufferManager();
    ~BufferManager();

    Buf* GetBlk(int blkno);	            /* 申请一块缓存，用于读写文件中的字符块blkno。*/
    void RelseB(Buf* bp);				/* 释放缓存控制块buf */

    /* 读一个磁盘块，blkno为目标磁盘块逻辑块号。 */
    Buffer* ReadB(int blkno);

    /* 写一个磁盘块 */
    void WriteB(Buffer* bp);

    /* 异步写一个磁盘块 */
    void AWriteB(Buffer* bp);

    /* 延迟写磁盘块 */
    void DWriteB(Buffer* bp);

    /* 清空缓冲区内容 */
    void ClearB(Buffer* bp);

    /* 将队列中延迟写的缓存全部输出到磁盘 */
    void FlushB();

    /* 获取空闲控制块Buf对象引用 */
    //Buffer& GetFreeBuf();				

    /* 格式化所有Buffer */
    void FormatBuffer();

private:
    void InitList();
    void DetachNode(Buffer* pb);
    void InsertTail(Buffer* pb);
    void debug();
};

#endif