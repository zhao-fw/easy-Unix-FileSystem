#include "Utility.h"
#include "OpenFileManager.h"
#include "User.h"

extern BufferManager g_BufferManager;
extern FileSystem g_FileSystem;
extern User g_User;

/*==============================class OpenFileTable===================================*/
/* 系统全局打开文件表对象实例的定义 */
extern OpenFileTable g_OpenFileTable;

OpenFileTable::OpenFileTable()
{
	//nothing to do here
}

OpenFileTable::~OpenFileTable()
{
	//nothing to do here
}

File* OpenFileTable::AllocF()
{
	int fd;
	User& u = g_User;

	/* 在进程打开文件描述符表中获取一个空闲项 */
	fd = u.u_ofiles.AllocFreeSlot();

	if (fd < 0)	/* 如果寻找空闲项失败 */
	{
		return NULL;
	}

	for (int i = 0; i < OpenFileTable::NFILE; i++)
	{
		/* f_count==0表示该项空闲 */
		if (this->m_File[i].f_count == 0)
		{
			/* 建立描述符和File结构的勾连关系 */
			u.u_ofiles.SetF(fd, &this->m_File[i]);
			/* 增加对file结构的引用计数 */
			this->m_File[i].f_count++;
			/* 清空文件读、写位置 */
			this->m_File[i].f_offset = 0;
			return (&this->m_File[i]);
		}
	}

	printf("No Free File Struct\n");
	u.u_error = User::U_ENFILE;
	return NULL;
}

void OpenFileTable::CloseF(File* pFile)
{
	if (pFile->f_count <= 1)
	{
		/*
		 * 如果当前进程是最后一个引用该文件的进程
		 */
		g_InodeTable.PutI(pFile->f_inode);
	}

	/* 引用当前File的进程数减1 */
	pFile->f_count--;
}

void OpenFileTable::Format() {
	File emptyFile;
	for (int i = 0; i < OpenFileTable::NFILE; ++i) {
		memcpy(m_File + i, &emptyFile, sizeof(File));
	}
}

/*==============================class InodeTable===================================*/
/*  定义内存Inode表的实例 */
extern InodeTable g_InodeTable;

InodeTable::InodeTable()
{
	//nothing to do here
}

InodeTable::~InodeTable()
{
	//nothing to do here
}

void InodeTable::Initialize()
{
	/* 获取对g_FileSystem的引用 */
	this->m_FileSystem = &g_FileSystem;
}

void InodeTable::Format() {
	Inode emptyINode;
	for (int i = 0; i < InodeTable::NINODE; ++i) {
		memcpy(m_Inode + i, &emptyINode, sizeof(Inode));
	}
}

Inode* InodeTable::GetI(int inumber)
{
	Inode* pInode;
	User& u = g_User;

	while (true)
	{
		/* 检查编号为inumber的外存Inode是否有内存拷贝 */
		int index = this->IsLoaded(inumber);
		if (index >= 0)	/* 找到内存拷贝 */
		{
			pInode = &(this->m_Inode[index]);
			pInode->i_count++;
			return pInode;
		}
		else	/* 没有Inode的内存拷贝，则分配一个空闲内存Inode */
		{
			pInode = this->GetFreeInode();
			/* 若内存Inode表已满，分配空闲Inode失败 */
			if (NULL == pInode)
			{
				printf("Inode Table Overflow !\n");
				u.u_error = User::U_ENFILE;
				return NULL;
			}
			else	/* 分配空闲Inode成功，将外存Inode读入新分配的内存Inode */
			{
				/* 设置新的外存Inode编号，增加引用计数，对索引节点上锁 */
				pInode->i_number = inumber;
				pInode->i_count++;

				BufferManager& bm = g_BufferManager;
				/* 将该外存Inode读入缓冲区 */
				Buf* pBuf = bm.ReadB(FileSystem::INODE_ZONE_START_SECTOR + inumber / FileSystem::INODE_NUMBER_PER_SECTOR);

				/* 如果发生I/O错误 */
				if (pBuf->b_flags & Buf::B_ERROR)
				{
					/* 释放缓存 */
					bm.RelseB(pBuf);
					/* 释放占据的内存Inode */
					this->PutI(pInode);
					return NULL;
				}

				/* 将缓冲区中的外存Inode信息拷贝到新分配的内存Inode中 */
				pInode->CopyI(pBuf, inumber);
				/* 释放缓存 */
				bm.RelseB(pBuf);
				return pInode;
			}
		}
	}
	return NULL;	/* GCC likes it! */
}

void InodeTable::PutI(Inode* pNode)
{
	/* 当前进程为引用该内存Inode的唯一进程，且准备释放该内存Inode */
	if (pNode->i_count == 1)
	{
		/* 该文件已经没有目录路径指向它 */
		if (pNode->i_nlink <= 0)
		{
			/* 释放该文件占据的数据盘块 */
			pNode->TruncI();
			pNode->i_mode = 0;
			/* 释放对应的外存Inode */
			this->m_FileSystem->FreeI(pNode->i_number);
		}

		/* 更新外存Inode信息 */
		pNode->UpdateI((int)Utility::time(NULL));

		/* 清除内存Inode的所有标志位 */
		pNode->i_flag = 0;
		/* 这是内存inode空闲的标志之一，另一个是i_count == 0 */
		pNode->i_number = -1;
	}

	/* 减少内存Inode的引用计数，唤醒等待进程 */
	pNode->i_count--;
}

void InodeTable::UpdateInodeTable()
{
	for (int i = 0; i < InodeTable::NINODE; i++)
	{
		/*
		 * 如果Inode对象没有被上锁，即当前未被其它进程使用，可以同步到外存Inode；
		 * 并且count不等于0，count == 0意味着该内存Inode未被任何打开文件引用，无需同步。
		 */
		if (this->m_Inode[i].i_count != 0)
		{
			/* 将内存Inode上锁后同步到外存Inode */
			// this->m_Inode[i].i_flag |= Inode::ILOCK;
			this->m_Inode[i].UpdateI((int)Utility::time(NULL));

			/* 对内存Inode解锁 */
			// this->m_Inode[i].Prele();
		}
	}
}

int InodeTable::IsLoaded(int inumber)
{
	/* 寻找指定外存Inode的内存拷贝 */
	for (int i = 0; i < InodeTable::NINODE; i++)
	{
		if (this->m_Inode[i].i_number == inumber && this->m_Inode[i].i_count != 0)
		{
			return i;
		}
	}
	return -1;
}

Inode* InodeTable::GetFreeInode()
{
	for (int i = 0; i < InodeTable::NINODE; i++)
	{
		/* 如果该内存Inode引用计数为零，则该Inode表示空闲 */
		if (this->m_Inode[i].i_count == 0)
		{
			return &(this->m_Inode[i]);
		}
	}
	return NULL;	/* 寻找失败 */
}
