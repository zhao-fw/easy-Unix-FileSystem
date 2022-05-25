#ifndef BUFFERMANAGER_H
#define BUFFERMANAGER_H

#include"Buf.h"
#include"Device.h"


class BufferManager {
public:
    static const int NBUF = 100;			/* ������ƿ顢������������ */
    static const int BUFFER_SIZE = 512;		/* ��������С�� ���ֽ�Ϊ��λ */

private:
    Buf* bufferList;							/* ������п��ƿ� */
    Buf nBuffer[NBUF];						/* ������ƿ����� */
    unsigned char Buffer[NBUF][BUFFER_SIZE];	/* ���������� */
    unordered_map<int, Buf*> map;
    Device* device;

public:
    BufferManager();
    ~BufferManager();

    Buf* GetBlk(int blkno);	            /* ����һ�黺�棬���ڶ�д�ļ��е��ַ���blkno��*/
    void RelseB(Buf* bp);				/* �ͷŻ�����ƿ�buf */

    /* ��һ�����̿飬blknoΪĿ����̿��߼���š� */
    Buffer* ReadB(int blkno);

    /* дһ�����̿� */
    void WriteB(Buffer* bp);

    /* �첽дһ�����̿� */
    void AWriteB(Buffer* bp);

    /* �ӳ�д���̿� */
    void DWriteB(Buffer* bp);

    /* ��ջ��������� */
    void ClearB(Buffer* bp);

    /* ���������ӳ�д�Ļ���ȫ����������� */
    void FlushB();

    /* ��ȡ���п��ƿ�Buf�������� */
    //Buffer& GetFreeBuf();				

    /* ��ʽ������Buffer */
    void FormatBuffer();

private:
    void InitList();
    void DetachNode(Buffer* pb);
    void InsertTail(Buffer* pb);
    void debug();
};

#endif