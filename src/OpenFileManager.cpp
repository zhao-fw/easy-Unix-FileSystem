#include "Utility.h"
#include "OpenFileManager.h"
#include "User.h"

extern BufferManager g_BufferManager;
extern FileSystem g_FileSystem;
extern User g_User;

/*==============================class OpenFileTable===================================*/
/* ϵͳȫ�ִ��ļ������ʵ���Ķ��� */
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

	/* �ڽ��̴��ļ����������л�ȡһ�������� */
	fd = u.u_ofiles.AllocFreeSlot();

	if (fd < 0)	/* ���Ѱ�ҿ�����ʧ�� */
	{
		return NULL;
	}

	for (int i = 0; i < OpenFileTable::NFILE; i++)
	{
		/* f_count==0��ʾ������� */
		if (this->m_File[i].f_count == 0)
		{
			/* ������������File�ṹ�Ĺ�����ϵ */
			u.u_ofiles.SetF(fd, &this->m_File[i]);
			/* ���Ӷ�file�ṹ�����ü��� */
			this->m_File[i].f_count++;
			/* ����ļ�����дλ�� */
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
		 * �����ǰ���������һ�����ø��ļ��Ľ���
		 */
		g_InodeTable.PutI(pFile->f_inode);
	}

	/* ���õ�ǰFile�Ľ�������1 */
	pFile->f_count--;
}

void OpenFileTable::Format() {
	File emptyFile;
	for (int i = 0; i < OpenFileTable::NFILE; ++i) {
		memcpy(m_File + i, &emptyFile, sizeof(File));
	}
}

/*==============================class InodeTable===================================*/
/*  �����ڴ�Inode���ʵ�� */
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
	/* ��ȡ��g_FileSystem������ */
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
		/* �����Ϊinumber�����Inode�Ƿ����ڴ濽�� */
		int index = this->IsLoaded(inumber);
		if (index >= 0)	/* �ҵ��ڴ濽�� */
		{
			pInode = &(this->m_Inode[index]);
			pInode->i_count++;
			return pInode;
		}
		else	/* û��Inode���ڴ濽���������һ�������ڴ�Inode */
		{
			pInode = this->GetFreeInode();
			/* ���ڴ�Inode���������������Inodeʧ�� */
			if (NULL == pInode)
			{
				printf("Inode Table Overflow !\n");
				u.u_error = User::U_ENFILE;
				return NULL;
			}
			else	/* �������Inode�ɹ��������Inode�����·�����ڴ�Inode */
			{
				/* �����µ����Inode��ţ��������ü������������ڵ����� */
				pInode->i_number = inumber;
				pInode->i_count++;

				BufferManager& bm = g_BufferManager;
				/* �������Inode���뻺���� */
				Buf* pBuf = bm.ReadB(FileSystem::INODE_ZONE_START_SECTOR + inumber / FileSystem::INODE_NUMBER_PER_SECTOR);

				/* �������I/O���� */
				if (pBuf->b_flags & Buf::B_ERROR)
				{
					/* �ͷŻ��� */
					bm.RelseB(pBuf);
					/* �ͷ�ռ�ݵ��ڴ�Inode */
					this->PutI(pInode);
					return NULL;
				}

				/* ���������е����Inode��Ϣ�������·�����ڴ�Inode�� */
				pInode->CopyI(pBuf, inumber);
				/* �ͷŻ��� */
				bm.RelseB(pBuf);
				return pInode;
			}
		}
	}
	return NULL;	/* GCC likes it! */
}

void InodeTable::PutI(Inode* pNode)
{
	/* ��ǰ����Ϊ���ø��ڴ�Inode��Ψһ���̣���׼���ͷŸ��ڴ�Inode */
	if (pNode->i_count == 1)
	{
		/* ���ļ��Ѿ�û��Ŀ¼·��ָ���� */
		if (pNode->i_nlink <= 0)
		{
			/* �ͷŸ��ļ�ռ�ݵ������̿� */
			pNode->TruncI();
			pNode->i_mode = 0;
			/* �ͷŶ�Ӧ�����Inode */
			this->m_FileSystem->FreeI(pNode->i_number);
		}

		/* �������Inode��Ϣ */
		pNode->UpdateI((int)Utility::time(NULL));

		/* ����ڴ�Inode�����б�־λ */
		pNode->i_flag = 0;
		/* �����ڴ�inode���еı�־֮һ����һ����i_count == 0 */
		pNode->i_number = -1;
	}

	/* �����ڴ�Inode�����ü��������ѵȴ����� */
	pNode->i_count--;
}

void InodeTable::UpdateInodeTable()
{
	for (int i = 0; i < InodeTable::NINODE; i++)
	{
		/*
		 * ���Inode����û�б�����������ǰδ����������ʹ�ã�����ͬ�������Inode��
		 * ����count������0��count == 0��ζ�Ÿ��ڴ�Inodeδ���κδ��ļ����ã�����ͬ����
		 */
		if (this->m_Inode[i].i_count != 0)
		{
			/* ���ڴ�Inode������ͬ�������Inode */
			// this->m_Inode[i].i_flag |= Inode::ILOCK;
			this->m_Inode[i].UpdateI((int)Utility::time(NULL));

			/* ���ڴ�Inode���� */
			// this->m_Inode[i].Prele();
		}
	}
}

int InodeTable::IsLoaded(int inumber)
{
	/* Ѱ��ָ�����Inode���ڴ濽�� */
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
		/* ������ڴ�Inode���ü���Ϊ�㣬���Inode��ʾ���� */
		if (this->m_Inode[i].i_count == 0)
		{
			return &(this->m_Inode[i]);
		}
	}
	return NULL;	/* Ѱ��ʧ�� */
}
