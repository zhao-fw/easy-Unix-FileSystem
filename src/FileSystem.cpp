#include "FileSystem.h"
#include "Utility.h"
#include "OpenFileManager.h"
#include "User.h"
#include<windows.h>

/*
 * 由于不需要实现kernel等内容，所以对于在kernel中定义的 g_BufferManager, g_FileSystem, g_user等内容，
 * 直接在main中定义的全局变量来实现。
 */
extern BufferManager g_BufferManager;
extern InodeTable g_INodeTable; 
extern FileSystem g_FileSystem;
extern Device g_Device;
extern User g_User;

/*==============================class SuperBlock===================================*/
/* 系统全局超级块SuperBlock对象 */
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
	/* 如果该SuperBlock内存副本没有被修改，直接管理inode和空闲盘块被上锁或该文件系统是只读文件系统 */
	if (m_SuperBlock->s_fmod != 0 && m_SuperBlock->s_ilock == 0 && m_SuperBlock->s_flock == 0 && m_SuperBlock->s_ronly == 0)
	{
		/* 清SuperBlock修改标志 */
		m_SuperBlock->s_fmod = 0;
		/* 写入SuperBlock最后存访时间 */
		m_SuperBlock->s_time = (int)Utility::time(NULL);

		/*
		 * 为将要写回到磁盘上去的SuperBlock申请一块缓存，由于缓存块大小为512字节，
		 * SuperBlock大小为1024字节，占据2个连续的扇区，所以需要2次写入操作。
		 */
		for (int j = 0; j < 2; j++)
		{
			/* 第一次p指向SuperBlock的第0字节，第二次p指向第512字节 */
			int* p = (int*)m_SuperBlock + j * 128;

			/* 将要写入到设备dev上的SUPER_BLOCK_SECTOR_NUMBER + j扇区中去 */
			pBuf = this->m_BufferManager->GetBlk(FileSystem::SUPER_BLOCK_START_SECTOR + j);

			/* 将SuperBlock中第0 - 511字节写入缓存区 */
			Utility::DWordCopy(p, (int*)pBuf->b_addr, 128);

			/* 将缓冲区中的数据写到磁盘上 */
			this->m_BufferManager->WriteB(pBuf);
		}
	}
	/* 同步修改过的内存Inode到对应外存Inode */
	g_InodeTable.UpdateInodeTable();

	/* 将延迟写的缓存块写到磁盘上 */
	this->m_BufferManager->FlushB();
}

/* 格式化SuperBlock */
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

/* 格式化整个文件系统 */
void FileSystem::FormatDevice() {
	FormatSuperBlock();
	m_Device->NewD();

	//空文件，先写入superblock占据空间，未设置文件大小
	m_Device->WriteD(m_SuperBlock, sizeof(SuperBlock), 0);

	DiskInode emptyDInode, rootDInode;
	//根目录DiskNode
	rootDInode.d_mode |= Inode::IALLOC | Inode::IFDIR;
	rootDInode.d_nlink = 1;
	m_Device->WriteD(&rootDInode, sizeof(rootDInode));

	//从第1个DiskINode初始化，第0个固定用于根目录"/"，不可改变
	for (int i = 1; i < FileSystem::INODE_ZONE_SIZE * FileSystem::INODE_NUMBER_PER_SECTOR; ++i) {
		if (m_SuperBlock->s_ninode < 100) {
			m_SuperBlock->s_inode[m_SuperBlock->s_ninode++] = i;
		}
		m_Device->WriteD(&emptyDInode, sizeof(emptyDInode));
	}

	//空闲盘块初始化
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
	//再次写入superblock
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
	int ino;	/* 分配到的空闲外存Inode编号 */

	/*
	 * SuperBlock直接管理的空闲Inode索引表已空，
	 * 必须到磁盘上搜索空闲Inode。先对inode列表上锁，
	 * 因为在以下程序中会进行读盘操作可能会导致进程切换，
	 * 其他进程有可能访问该索引表，将会导致不一致性。
	 */
	if (m_SuperBlock->s_ninode <= 0)
	{
		/* 空闲Inode索引表上锁 */
		m_SuperBlock->s_ilock++;

		/* 外存Inode编号从0开始，这不同于Unix V6中外存Inode从1开始编号 */
		ino = -1;

		/* 依次读入磁盘Inode区中的磁盘块，搜索其中空闲外存Inode，记入空闲Inode索引表 */
		for (int i = 0; i < m_SuperBlock->s_isize; i++)
		{
			pBuf = this->m_BufferManager->ReadB(FileSystem::INODE_ZONE_START_SECTOR + i);

			/* 获取缓冲区首址 */
			int* p = (int*)pBuf->b_addr;

			/* 检查该缓冲区中每个外存Inode的i_mode != 0，表示已经被占用 */
			for (int j = 0; j < FileSystem::INODE_NUMBER_PER_SECTOR; j++)
			{
				ino++;

				int mode = *(p + j * sizeof(DiskInode) / sizeof(int));

				/* 该外存Inode已被占用，不能记入空闲Inode索引表 */
				if (mode != 0)
				{
					continue;
				}

				/*
				 * 如果外存inode的i_mode==0，此时并不能确定
				 * 该inode是空闲的，因为有可能是内存inode没有写到
				 * 磁盘上,所以要继续搜索内存inode中是否有相应的项
				 */
				if (g_InodeTable.IsLoaded(ino) == -1)
				{
					/* 该外存Inode没有对应的内存拷贝，将其记入空闲Inode索引表 */
					m_SuperBlock->s_inode[m_SuperBlock->s_ninode++] = ino;

					/* 如果空闲索引表已经装满，则不继续搜索 */
					if (m_SuperBlock->s_ninode >= 100)
					{
						break;
					}
				}
			}

			/* 至此已读完当前磁盘块，释放相应的缓存 */
			this->m_BufferManager->RelseB(pBuf);

			/* 如果空闲索引表已经装满，则不继续搜索 */
			if (m_SuperBlock->s_ninode >= 100)
			{
				break;
			}
		}
		/* 解除对空闲外存Inode索引表的锁，唤醒因为等待锁而睡眠的进程 */
		m_SuperBlock->s_ilock = 0;
		// Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&m_SuperBlock->s_flock);

		/* 如果在磁盘上没有搜索到任何可用外存Inode，返回NULL */
		if (m_SuperBlock->s_ninode <= 0)
		{
			printf("磁盘没有空闲的Inode\n");
			u.u_error = User::U_ENOSPC;
			return NULL;
		}
	}

	/*
	 * 上面部分已经保证，除非系统中没有可用外存Inode，
	 * 否则空闲Inode索引表中必定会记录可用外存Inode的编号。
	 */
	while (true)
	{
		/* 从索引表“栈顶”获取空闲外存Inode编号 */
		ino = m_SuperBlock->s_inode[--m_SuperBlock->s_ninode];

		/* 将空闲Inode读入内存 */
		pNode = g_InodeTable.GetI(ino);
		/* 未能分配到内存inode */
		if (NULL == pNode)
		{
			printf("AllocI时，无可用的内存Inode\n");
			return NULL;
		}

		/* 如果该Inode空闲,清空Inode中的数据 */
		if (0 == pNode->i_mode)
		{
			pNode->CleanI();
			/* 设置SuperBlock被修改标志 */
			m_SuperBlock->s_fmod = 1;
			return pNode;
		}
		else	/* 如果该Inode已被占用 */
		{
			g_InodeTable.PutI(pNode);
			continue;	/* while循环 */
		}
	}
	return NULL;	/* GCC likes it! */
}

void FileSystem::FreeI(int number)
{
	/*
	 * 如果超级块直接管理的空闲Inode表上锁，
	 * 则释放的外存Inode散落在磁盘Inode区中。
	 */
	if (m_SuperBlock->s_ilock)
	{
		return;
	}

	/*
	 * 如果超级块直接管理的空闲外存Inode超过100个，
	 * 同样让释放的外存Inode散落在磁盘Inode区中。
	 */
	if (m_SuperBlock->s_ninode >= 100)
	{
		return;
	}

	m_SuperBlock->s_inode[m_SuperBlock->s_ninode++] = number;

	/* 设置SuperBlock被修改标志 */
	m_SuperBlock->s_fmod = 1;
}

Buf* FileSystem::AllocBlock()
{
	int blkno;	/* 分配到的空闲磁盘块编号 */
	Buf* pBuf;
	User& u = g_User;

	/*
	 * 如果空闲磁盘块索引表正在被上锁，表明有其它进程
	 * 正在操作空闲磁盘块索引表，因而对其上锁。这通常
	 * 是由于其余进程调用Free()或Alloc()造成的。
	 */
	while (m_SuperBlock->s_flock)
	{
		/* 进入睡眠直到获得该锁才继续 */
		// u.u_procp->Sleep((unsigned long)&m_SuperBlock->s_flock, ProcessManager::PINOD);
		Sleep(1000);
	}

	/* 从索引表“栈顶”获取空闲磁盘块编号 */
	blkno = m_SuperBlock->s_free[--m_SuperBlock->s_nfree];

	/*
	 * 若获取磁盘块编号为零，则表示已分配尽所有的空闲磁盘块。
	 * 或者分配到的空闲磁盘块编号不属于数据盘块区域中(由BadBlock()检查)，
	 * 都意味着分配空闲磁盘块操作失败。
	 */
	if (0 == blkno)
	{
		m_SuperBlock->s_nfree = 0;
		printf("磁盘上没有空闲的磁盘块\n");
		u.u_error = User::U_ENOSPC;
		return NULL;
	}
	if (this->BadBlock(m_SuperBlock, blkno))
	{
		return NULL;
	}

	/*
	 * 栈已空，新分配到空闲磁盘块中记录了下一组空闲磁盘块的编号,
	 * 将下一组空闲磁盘块的编号读入SuperBlock的空闲磁盘块索引表s_free[100]中。
	 */
	if (m_SuperBlock->s_nfree <= 0)
	{
		/*
		 * 此处加锁，因为以下要进行读盘操作，有可能发生进程切换，
		 * 新上台的进程可能对SuperBlock的空闲盘块索引表访问，会导致不一致性。
		 */
		m_SuperBlock->s_flock++;

		/* 读入该空闲磁盘块 */
		pBuf = this->m_BufferManager->ReadB(blkno);

		/* 从该磁盘块的0字节开始记录，共占据4(s_nfree)+400(s_free[100])个字节 */
		int* p = (int*)pBuf->b_addr;

		/* 首先读出空闲盘块数s_nfree */
		m_SuperBlock->s_nfree = *p++;

		/* 读取缓存中后续位置的数据，写入到SuperBlock空闲盘块索引表s_free[100]中 */
		Utility::DWordCopy(p, m_SuperBlock->s_free, 100);

		/* 缓存使用完毕，释放以便被其它进程使用 */
		this->m_BufferManager->RelseB(pBuf);

		/* 解除对空闲磁盘块索引表的锁，唤醒因为等待锁而睡眠的进程 */
		m_SuperBlock->s_flock = 0;
		// Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&m_SuperBlock->s_flock);
	}

	/* 普通情况下成功分配到一空闲磁盘块 */
	pBuf = this->m_BufferManager->GetBlk(blkno);	/* 为该磁盘块申请缓存 */
	this->m_BufferManager->ClearB(pBuf);			/* 清空缓存中的数据 */
	m_SuperBlock->s_fmod = 1;	/* 设置SuperBlock被修改标志 */

	return pBuf;
}

void FileSystem::FreeBlock(int blkno)
{
	Buf* pBuf;
	User& u = g_User;

	/*
	 * 尽早设置SuperBlock被修改标志，以防止在释放
	 * 磁盘块Free()执行过程中，对SuperBlock内存副本
	 * 的修改仅进行了一半，就更新到磁盘SuperBlock去
	 */
	m_SuperBlock->s_fmod = 1;

	/* 如果空闲磁盘块索引表被上锁，则睡眠等待解锁 */
	while (m_SuperBlock->s_flock)
	{
		/* 进入睡眠直到获得该锁才继续 */
		// u.u_procp->Sleep((unsigned long)&m_SuperBlock->s_flock, ProcessManager::PINOD);
		Sleep(1000);
	}

	/* 检查释放磁盘块的合法性 */
	if (this->BadBlock(m_SuperBlock, blkno))
	{
		return;
	}

	/*
	 * 如果先前系统中已经没有空闲盘块，
	 * 现在释放的是系统中第1块空闲盘块
	 */
	if (m_SuperBlock->s_nfree <= 0)
	{
		m_SuperBlock->s_nfree = 1;
		m_SuperBlock->s_free[0] = 0;	/* 使用0标记空闲盘块链结束标志 */
	}

	/* SuperBlock中直接管理空闲磁盘块号的栈已满 */
	if (m_SuperBlock->s_nfree >= 100)
	{
		m_SuperBlock->s_flock++;

		/*
		 * 使用当前Free()函数正要释放的磁盘块，存放前一组100个空闲
		 * 磁盘块的索引表
		 */
		pBuf = this->m_BufferManager->GetBlk(blkno);	/* 为当前正要释放的磁盘块分配缓存 */

		/* 从该磁盘块的0字节开始记录，共占据4(s_nfree)+400(s_free[100])个字节 */
		int* p = (int*)pBuf->b_addr;

		/* 首先写入空闲盘块数，除了第一组为99块，后续每组都是100块 */
		*p++ = m_SuperBlock->s_nfree;
		/* 将SuperBlock的空闲盘块索引表s_free[100]写入缓存中后续位置 */
		Utility::DWordCopy(m_SuperBlock->s_free, p, 100);

		m_SuperBlock->s_nfree = 0;
		/* 将存放空闲盘块索引表的“当前释放盘块”写入磁盘，即实现了空闲盘块记录空闲盘块号的目标 */
		this->m_BufferManager->WriteB(pBuf);

		m_SuperBlock->s_flock = 0;
		// Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&m_SuperBlock->s_flock);
	}
	m_SuperBlock->s_free[m_SuperBlock->s_nfree++] = blkno;	/* SuperBlock中记录下当前释放盘块号 */
	m_SuperBlock->s_fmod = 1;
}

bool FileSystem::BadBlock(SuperBlock* spb, int blkno)
{
	return 0;
}
