#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "BufferManager.h"
#include "Utility.h"
#include <iostream>

extern Device g_Device;

/*
 *	Buffer只用到了两个标志，B_DONE和B_DELWRI，分别表示已经完成IO和延迟写的标志。
 *	空闲Buffer无任何标志
*/
BufferManager::BufferManager() {
	bFreeList = new Buf;
	m_device = &g_Device;
	InitList();
}

BufferManager::~BufferManager() {
	FlushB();
	delete bFreeList;
}

void BufferManager::FormatBuf() {
	Buf* tem = new Buf();
	for (int i = 0; i < NBUF; ++i) {
		std::memcpy(m_Buf + i, tem, sizeof(Buf));
	}
	InitList();
	delete tem;
}

void BufferManager::InitList() {
	for (int i = 0; i < NBUF; ++i) {
		if (i == 0) {
			m_Buf[i].av_forw = bFreeList;
			bFreeList->av_back = m_Buf + i;
		}
		else {
			m_Buf[i].av_forw = m_Buf + i - 1;
		}

		if (i == NBUF - 1) {
			m_Buf[i].av_back = bFreeList;
			bFreeList->av_forw = m_Buf + i;
		}
		else {
			m_Buf[i].av_back = m_Buf + i + 1;
		}
		m_Buf[i].b_addr = Buffer[i];
		m_Buf[i].no = i;
	}
}

Buf* BufferManager::GetBlk(int blkno) {
	Buf* bp = NULL;
	bool find_it = false;
	for (int i = 0; i < NBUF; ++i) {
		if (m_Buf[i].b_blkno == blkno) {
			bp = m_Buf + i;
			find_it = true;
		}
	}
	
	if (!find_it) {
		if (bFreeList->av_back == bFreeList) {
			for (int i = 0; i < NBUF; ++i) {
				bp = m_Buf + i;
				if (bp->b_flags & Buf::B_DELWRI) {
					bp->delFlag(Buf::B_DELWRI);
					m_device->WriteD(bp->b_addr, BUFFER_SIZE, bp->b_blkno * BUFFER_SIZE);
					bp->b_blkno = blkno;
					bp->clrFlag();
					return bp;
				}
			}
		}
		bp = bFreeList->av_back;
	}

	if (bp && bp->av_back != NULL) {
		bp->av_forw->av_back = bp->av_back;
		bp->av_back->av_forw = bp->av_forw;
		bp->av_back = NULL;
		bp->av_forw = NULL;
	}

	if (bp->b_flags & Buf::B_DELWRI) {
		m_device->WriteD(bp->b_addr, BUFFER_SIZE, bp->b_blkno * BUFFER_SIZE);
	}
	bp->b_blkno = blkno;
	/* 清楚所有用到的标志位 */
	bp->clrFlag();
	return bp;
}

void BufferManager::RelseB(Buf* bp) {
	if (bp->av_back != NULL) {
		return;
	}
	bp->av_forw = bFreeList->av_forw;
	bp->av_back = bFreeList;
	bFreeList->av_forw->av_back = bp;
	bFreeList->av_forw = bp;
}

Buf* BufferManager::ReadB(int blkno) {
	Buf* bp = GetBlk(blkno);

	if (bp->b_flags & Buf::B_DONE) {
		return bp;
	}
	m_device->ReadD(bp->b_addr, BUFFER_SIZE, bp->b_blkno * BUFFER_SIZE);
	bp->addFlag(Buf::B_DONE);
	return bp;
}

void BufferManager::WriteB(Buf* bp) {
	bp->clrFlag();
	m_device->WriteD(bp->b_addr, BUFFER_SIZE, bp->b_blkno * BUFFER_SIZE);
	bp->addFlag(Buf::B_DONE);
	RelseB(bp);
}

void BufferManager::DWriteB(Buf* bp) {
	bp->addFlag(Buf::B_DELWRI);
	bp->addFlag(Buf::B_DONE);
	this->RelseB(bp);
}

void BufferManager::ClearB(Buf* bp) {
	std::memset(bp->b_addr, 0, BUFFER_SIZE);
}

void BufferManager::FlushB() {
	Buf* bp = NULL;
	for (int i = 0; i < NBUF; ++i) {
		bp = m_Buf + i;
		if (bp->b_flags & Buf::B_DELWRI) {
			bp->delFlag(Buf::B_DELWRI);
			m_device->WriteD(bp->b_addr, BUFFER_SIZE, bp->b_blkno * BUFFER_SIZE);
			bp->addFlag(Buf::B_DONE);
		}
	}
}

#endif