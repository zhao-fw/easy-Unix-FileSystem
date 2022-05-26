#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include"Buf.h"
#include"Device.h"


class BufferManager {
public:
    static const int NBUF = 100;			/* ������ƿ顢������������ */
    static const int BUFFER_SIZE = 512;		/* ��������С�� ���ֽ�Ϊ��λ */

private:
    Buf* bFreeList;							    /* ������п��ƿ� */
    Buf m_Buf[NBUF];						    /* ������ƿ����� */
    unsigned char Buffer[NBUF][BUFFER_SIZE];	/* ���������� */
    Device* m_device;

public:
    BufferManager();
    ~BufferManager();

    Buf* GetBlk(int blkno);         /* ����һ�黺�棬���ڶ�д�ļ��е��ַ���blkno��*/
    void RelseB(Buf* bp);           /* �ͷŻ�����ƿ�buf */

    Buf* ReadB(int blkno);          /* ��һ�����̿飬blknoΪĿ����̿��߼���š� */
    void WriteB(Buf* bp);           /* дһ�����̿� */
    void DWriteB(Buf* bp);          /* �ӳ�д���̿� */

    void ClearB(Buf* bp);           /* ��ջ��������� */
    void FlushB();                  /* ���������ӳ�д�Ļ���ȫ����������� */
    void FormatBuf();               /* ��ʽ������Buffer */

private:
    void InitList();
    void debug();
};

#endif