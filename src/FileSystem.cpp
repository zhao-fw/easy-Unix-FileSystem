#include "FileSystem.h"
#include "Utility.h"
#include "OpenFileManager.h"
#include "User.h"
#include<windows.h>

/*
 * ���ڲ���Ҫʵ��kernel�����ݣ����Զ�����kernel�ж���� g_BufferManager, g_FileSystem, g_user�����ݣ�
 * ֱ����main�ж����ȫ�ֱ�����ʵ�֡�
 */
extern BufferManager g_BufferManager;
extern InodeTable g_INodeTable; 
extern FileSystem g_FileSystem;
extern Device g_Device;
extern User g_User;

/*==============================class SuperBlock===================================*/
/* ϵͳȫ�ֳ�����SuperBlock���� */
extern SuperBlock g_SuperBlock;

SuperBlock::SuperBlock()
{
	//nothing to do here
}

SuperBlock::~SuperBlock()
{
	//nothing to do here
}

/*==============================class FileSystem===================================*/
FileSystem::FileSystem()
{
	Initialize();
}

FileSystem::~FileSystem()
{
	Update();
}

void FileSystem::Initialize()
{
	m_SuperBlock = &g_SuperBlock;
	m_BufferManager = &g_BufferManager;
	m_Device = &g_Device;
	if (!m_Device->Exist()) {
		FormatDevice();
	}
	else {
		LoadSuperBlock();
	}
}

void FileSystem::Update()
{
	Buf* pBuf;
	/* �����SuperBlock�ڴ渱��û�б��޸ģ�ֱ�ӹ���inode�Ϳ����̿鱻��������ļ�ϵͳ��ֻ���ļ�ϵͳ */
	if (m_SuperBlock->s_fmod != 0 && m_SuperBlock->s_ilock == 0 && m_SuperBlock->s_flock == 0 && m_SuperBlock->s_ronly == 0)
	{
		/* ��SuperBlock�޸ı�־ */
		m_SuperBlock->s_fmod = 0;
		/* д��SuperBlock�����ʱ�� */
		m_SuperBlock->s_time = (int)Utility::time(NULL);

		/*
		 * Ϊ��Ҫд�ص�������ȥ��SuperBlock����һ�黺�棬���ڻ�����СΪ512�ֽڣ�
		 * SuperBlock��СΪ1024�ֽڣ�ռ��2��������������������Ҫ2��д�������
		 */
		for (int j = 0; j < 2; j++)
		{
			/* ��һ��pָ��SuperBlock�ĵ�0�ֽڣ��ڶ���pָ���512�ֽ� */
			int* p = (int*)m_SuperBlock + j * 128;

			/* ��Ҫд�뵽�豸dev�ϵ�SUPER_BLOCK_SECTOR_NUMBER + j������ȥ */
			pBuf = this->m_BufferManager->GetBlk(FileSystem::SUPER_BLOCK_START_SECTOR + j);

			/* ��SuperBlock�е�0 - 511�ֽ�д�뻺���� */
			Utility::DWordCopy(p, (int*)pBuf->b_addr, 128);

			/* ���������е�����д�������� */
			this->m_BufferManager->WriteB(pBuf);
		}
	}
	/* ͬ���޸Ĺ����ڴ�Inode����Ӧ���Inode */
	g_InodeTable.UpdateInodeTable();

	/* ���ӳ�д�Ļ����д�������� */
	this->m_BufferManager->FlushB();
}

/* ��ʽ��SuperBlock */
void FileSystem::FormatSuperBlock() {
	m_SuperBlock->s_isize = FileSystem::INODE_ZONE_SIZE;
	m_SuperBlock->s_fsize = FileSystem::BLOCK_NUM;
	m_SuperBlock->s_nfree = 0;
	m_SuperBlock->s_free[0] = -1;
	m_SuperBlock->s_ninode = 0;
	m_SuperBlock->s_flock = 0;
	m_SuperBlock->s_ilock = 0;
	m_SuperBlock->s_fmod = 0;
	m_SuperBlock->s_ronly = 0;
	time((time_t*)&m_SuperBlock->s_time);
}

/* ��ʽ�������ļ�ϵͳ */
void FileSystem::FormatDevice() {
	FormatSuperBlock();
	m_Device->NewD();

	//���ļ�����д��superblockռ�ݿռ䣬δ�����ļ���С
	m_Device->WriteD(m_SuperBlock, sizeof(SuperBlock), 0);

	DiskInode emptyDInode, rootDInode;
	//��Ŀ¼DiskNode
	rootDInode.d_mode |= Inode::IALLOC | Inode::IFDIR;
	rootDInode.d_nlink = 1;
	m_Device->WriteD(&rootDInode, sizeof(rootDInode));

	//�ӵ�1��DiskINode��ʼ������0���̶����ڸ�Ŀ¼"/"�����ɸı�
	for (int i = 1; i < FileSystem::INODE_ZONE_SIZE * FileSystem::INODE_NUMBER_PER_SECTOR; ++i) {
		if (m_SuperBlock->s_ninode < 100) {
			m_SuperBlock->s_inode[m_SuperBlock->s_ninode++] = i;
		}
		m_Device->WriteD(&emptyDInode, sizeof(emptyDInode));
	}

	//�����̿��ʼ��
	char freeBlock[BLOCK_SIZE], freeBlock1[BLOCK_SIZE];
	memset(freeBlock, 0, BLOCK_SIZE);
	memset(freeBlock1, 0, BLOCK_SIZE);

	for (int i = 0; i < FileSystem::DATA_ZONE_SIZE; ++i) {
		if (m_SuperBlock->s_nfree >= 100) {
			memcpy(freeBlock1, &m_SuperBlock->s_nfree, sizeof(int) + sizeof(m_SuperBlock->s_free));
			m_Device->WriteD(&freeBlock1, BLOCK_SIZE);
			m_SuperBlock->s_nfree = 0;
		}
		else {
			m_Device->WriteD(freeBlock, BLOCK_SIZE);
		}
		m_SuperBlock->s_free[m_SuperBlock->s_nfree++] = i + DATA_ZONE_START_SECTOR;
	}

	time((time_t*)&m_SuperBlock->s_time);
	//�ٴ�д��superblock
	m_Device->WriteD(m_SuperBlock, sizeof(SuperBlock), 0);
}

void FileSystem::LoadSuperBlock()
{
	Buf* pBuf;
	for (int i = 0; i < 2; i++)
	{
		int* p = (int*)&g_SuperBlock + i * 128;

		pBuf = m_BufferManager->ReadB(FileSystem::SUPER_BLOCK_START_SECTOR + i);

		Utility::DWordCopy((int*)pBuf->b_addr, p, 128);

		m_BufferManager->RelseB(pBuf);
	}
	g_SuperBlock.s_flock = 0;
	g_SuperBlock.s_ilock = 0;
	g_SuperBlock.s_ronly = 0;
	// g_SuperBlock.s_time = Utility::time(NULL);
}

Inode* FileSystem::AllocI()
{
	Buf* pBuf;
	Inode* pNode;
	User& u = g_User;
	int ino;	/* ���䵽�Ŀ������Inode��� */

	/*
	 * SuperBlockֱ�ӹ���Ŀ���Inode�������ѿգ�
	 * ���뵽��������������Inode���ȶ�inode�б�������
	 * ��Ϊ�����³����л���ж��̲������ܻᵼ�½����л���
	 * ���������п��ܷ��ʸ����������ᵼ�²�һ���ԡ�
	 */
	if (m_SuperBlock->s_ninode <= 0)
	{
		/* ����Inode���������� */
		m_SuperBlock->s_ilock++;

		/* ���Inode��Ŵ�0��ʼ���ⲻͬ��Unix V6�����Inode��1��ʼ��� */
		ino = -1;

		/* ���ζ������Inode���еĴ��̿飬�������п������Inode���������Inode������ */
		for (int i = 0; i < m_SuperBlock->s_isize; i++)
		{
			pBuf = this->m_BufferManager->ReadB(FileSystem::INODE_ZONE_START_SECTOR + i);

			/* ��ȡ��������ַ */
			int* p = (int*)pBuf->b_addr;

			/* ���û�������ÿ�����Inode��i_mode != 0����ʾ�Ѿ���ռ�� */
			for (int j = 0; j < FileSystem::INODE_NUMBER_PER_SECTOR; j++)
			{
				ino++;

				int mode = *(p + j * sizeof(DiskInode) / sizeof(int));

				/* �����Inode�ѱ�ռ�ã����ܼ������Inode������ */
				if (mode != 0)
				{
					continue;
				}

				/*
				 * ������inode��i_mode==0����ʱ������ȷ��
				 * ��inode�ǿ��еģ���Ϊ�п������ڴ�inodeû��д��
				 * ������,����Ҫ���������ڴ�inode���Ƿ�����Ӧ����
				 */
				if (g_InodeTable.IsLoaded(ino) == -1)
				{
					/* �����Inodeû�ж�Ӧ���ڴ濽��������������Inode������ */
					m_SuperBlock->s_inode[m_SuperBlock->s_ninode++] = ino;

					/* ��������������Ѿ�װ�����򲻼������� */
					if (m_SuperBlock->s_ninode >= 100)
					{
						break;
					}
				}
			}

			/* �����Ѷ��굱ǰ���̿飬�ͷ���Ӧ�Ļ��� */
			this->m_BufferManager->RelseB(pBuf);

			/* ��������������Ѿ�װ�����򲻼������� */
			if (m_SuperBlock->s_ninode >= 100)
			{
				break;
			}
		}
		/* ����Կ������Inode�����������������Ϊ�ȴ�����˯�ߵĽ��� */
		m_SuperBlock->s_ilock = 0;
		// Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&m_SuperBlock->s_flock);

		/* ����ڴ�����û���������κο������Inode������NULL */
		if (m_SuperBlock->s_ninode <= 0)
		{
			printf("����û�п��е�Inode\n");
			u.u_error = User::U_ENOSPC;
			return NULL;
		}
	}

	/*
	 * ���沿���Ѿ���֤������ϵͳ��û�п������Inode��
	 * �������Inode�������бض����¼�������Inode�ı�š�
	 */
	while (true)
	{
		/* ��������ջ������ȡ�������Inode��� */
		ino = m_SuperBlock->s_inode[--m_SuperBlock->s_ninode];

		/* ������Inode�����ڴ� */
		pNode = g_InodeTable.GetI(ino);
		/* δ�ܷ��䵽�ڴ�inode */
		if (NULL == pNode)
		{
			printf("AllocIʱ���޿��õ��ڴ�Inode\n");
			return NULL;
		}

		/* �����Inode����,���Inode�е����� */
		if (0 == pNode->i_mode)
		{
			pNode->CleanI();
			/* ����SuperBlock���޸ı�־ */
			m_SuperBlock->s_fmod = 1;
			return pNode;
		}
		else	/* �����Inode�ѱ�ռ�� */
		{
			g_InodeTable.PutI(pNode);
			continue;	/* whileѭ�� */
		}
	}
	return NULL;	/* GCC likes it! */
}

void FileSystem::FreeI(int number)
{
	/*
	 * ���������ֱ�ӹ���Ŀ���Inode��������
	 * ���ͷŵ����Inodeɢ���ڴ���Inode���С�
	 */
	if (m_SuperBlock->s_ilock)
	{
		return;
	}

	/*
	 * ���������ֱ�ӹ���Ŀ������Inode����100����
	 * ͬ�����ͷŵ����Inodeɢ���ڴ���Inode���С�
	 */
	if (m_SuperBlock->s_ninode >= 100)
	{
		return;
	}

	m_SuperBlock->s_inode[m_SuperBlock->s_ninode++] = number;

	/* ����SuperBlock���޸ı�־ */
	m_SuperBlock->s_fmod = 1;
}

Buf* FileSystem::AllocBlock()
{
	int blkno;	/* ���䵽�Ŀ��д��̿��� */
	Buf* pBuf;
	User& u = g_User;

	/*
	 * ������д��̿����������ڱ���������������������
	 * ���ڲ������д��̿����������������������ͨ��
	 * ������������̵���Free()��Alloc()��ɵġ�
	 */
	while (m_SuperBlock->s_flock)
	{
		/* ����˯��ֱ����ø����ż��� */
		// u.u_procp->Sleep((unsigned long)&m_SuperBlock->s_flock, ProcessManager::PINOD);
		Sleep(1000);
	}

	/* ��������ջ������ȡ���д��̿��� */
	blkno = m_SuperBlock->s_free[--m_SuperBlock->s_nfree];

	/*
	 * ����ȡ���̿���Ϊ�㣬���ʾ�ѷ��価���еĿ��д��̿顣
	 * ���߷��䵽�Ŀ��д��̿��Ų����������̿�������(��BadBlock()���)��
	 * ����ζ�ŷ�����д��̿����ʧ�ܡ�
	 */
	if (0 == blkno)
	{
		m_SuperBlock->s_nfree = 0;
		printf("������û�п��еĴ��̿�\n");
		u.u_error = User::U_ENOSPC;
		return NULL;
	}
	if (this->BadBlock(m_SuperBlock, blkno))
	{
		return NULL;
	}

	/*
	 * ջ�ѿգ��·��䵽���д��̿��м�¼����һ����д��̿�ı��,
	 * ����һ����д��̿�ı�Ŷ���SuperBlock�Ŀ��д��̿�������s_free[100]�С�
	 */
	if (m_SuperBlock->s_nfree <= 0)
	{
		/*
		 * �˴���������Ϊ����Ҫ���ж��̲������п��ܷ��������л���
		 * ����̨�Ľ��̿��ܶ�SuperBlock�Ŀ����̿���������ʣ��ᵼ�²�һ���ԡ�
		 */
		m_SuperBlock->s_flock++;

		/* ����ÿ��д��̿� */
		pBuf = this->m_BufferManager->ReadB(blkno);

		/* �Ӹô��̿��0�ֽڿ�ʼ��¼����ռ��4(s_nfree)+400(s_free[100])���ֽ� */
		int* p = (int*)pBuf->b_addr;

		/* ���ȶ��������̿���s_nfree */
		m_SuperBlock->s_nfree = *p++;

		/* ��ȡ�����к���λ�õ����ݣ�д�뵽SuperBlock�����̿�������s_free[100]�� */
		Utility::DWordCopy(p, m_SuperBlock->s_free, 100);

		/* ����ʹ����ϣ��ͷ��Ա㱻��������ʹ�� */
		this->m_BufferManager->RelseB(pBuf);

		/* ����Կ��д��̿������������������Ϊ�ȴ�����˯�ߵĽ��� */
		m_SuperBlock->s_flock = 0;
		// Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&m_SuperBlock->s_flock);
	}

	/* ��ͨ����³ɹ����䵽һ���д��̿� */
	pBuf = this->m_BufferManager->GetBlk(blkno);	/* Ϊ�ô��̿����뻺�� */
	this->m_BufferManager->ClearB(pBuf);			/* ��ջ����е����� */
	m_SuperBlock->s_fmod = 1;	/* ����SuperBlock���޸ı�־ */

	return pBuf;
}

void FileSystem::FreeBlock(int blkno)
{
	Buf* pBuf;
	User& u = g_User;

	/*
	 * ��������SuperBlock���޸ı�־���Է�ֹ���ͷ�
	 * ���̿�Free()ִ�й����У���SuperBlock�ڴ渱��
	 * ���޸Ľ�������һ�룬�͸��µ�����SuperBlockȥ
	 */
	m_SuperBlock->s_fmod = 1;

	/* ������д��̿���������������˯�ߵȴ����� */
	while (m_SuperBlock->s_flock)
	{
		/* ����˯��ֱ����ø����ż��� */
		// u.u_procp->Sleep((unsigned long)&m_SuperBlock->s_flock, ProcessManager::PINOD);
		Sleep(1000);
	}

	/* ����ͷŴ��̿�ĺϷ��� */
	if (this->BadBlock(m_SuperBlock, blkno))
	{
		return;
	}

	/*
	 * �����ǰϵͳ���Ѿ�û�п����̿飬
	 * �����ͷŵ���ϵͳ�е�1������̿�
	 */
	if (m_SuperBlock->s_nfree <= 0)
	{
		m_SuperBlock->s_nfree = 1;
		m_SuperBlock->s_free[0] = 0;	/* ʹ��0��ǿ����̿���������־ */
	}

	/* SuperBlock��ֱ�ӹ�����д��̿�ŵ�ջ���� */
	if (m_SuperBlock->s_nfree >= 100)
	{
		m_SuperBlock->s_flock++;

		/*
		 * ʹ�õ�ǰFree()������Ҫ�ͷŵĴ��̿飬���ǰһ��100������
		 * ���̿��������
		 */
		pBuf = this->m_BufferManager->GetBlk(blkno);	/* Ϊ��ǰ��Ҫ�ͷŵĴ��̿���仺�� */

		/* �Ӹô��̿��0�ֽڿ�ʼ��¼����ռ��4(s_nfree)+400(s_free[100])���ֽ� */
		int* p = (int*)pBuf->b_addr;

		/* ����д������̿��������˵�һ��Ϊ99�飬����ÿ�鶼��100�� */
		*p++ = m_SuperBlock->s_nfree;
		/* ��SuperBlock�Ŀ����̿�������s_free[100]д�뻺���к���λ�� */
		Utility::DWordCopy(m_SuperBlock->s_free, p, 100);

		m_SuperBlock->s_nfree = 0;
		/* ����ſ����̿�������ġ���ǰ�ͷ��̿顱д����̣���ʵ���˿����̿��¼�����̿�ŵ�Ŀ�� */
		this->m_BufferManager->WriteB(pBuf);

		m_SuperBlock->s_flock = 0;
		// Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&m_SuperBlock->s_flock);
	}
	m_SuperBlock->s_free[m_SuperBlock->s_nfree++] = blkno;	/* SuperBlock�м�¼�µ�ǰ�ͷ��̿�� */
	m_SuperBlock->s_fmod = 1;
}

bool FileSystem::BadBlock(SuperBlock* spb, int blkno)
{
	return 0;
}
