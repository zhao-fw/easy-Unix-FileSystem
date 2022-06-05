#include "FileManager.h"
#include "Utility.h"
#include "User.h"

extern BufferManager g_BufferManager;
extern FileSystem g_FileSystem;
extern InodeTable g_InodeTable;
extern OpenFileTable g_OpenFileTable;
extern User g_User;

/*==========================class FileManager===============================*/
FileManager::FileManager()
{
	//nothing to do here
	Initialize();
}

FileManager::~FileManager()
{
	//nothing to do here
}

void FileManager::Initialize()
{
	m_FileSystem = &g_FileSystem;
	m_InodeTable = &g_InodeTable;
	m_OpenFileTable = &g_OpenFileTable;
	m_InodeTable->Initialize();
	/* 对于root目录的设置，Unix V6++中是在main函数中设置的，这里为了方便起见，在Initial函数中设置 */
	rootDirInode = m_InodeTable->GetI(FileSystem::ROOTINO);
	rootDirInode->i_count += 0xff;
}

void FileManager::Open()
{
	Inode* pInode;
	User& u = g_User;

	pInode = this->NameI(NextChar, FileManager::OPEN);	/* 0 = Open, not create */
	/* 没有找到相应的Inode */
	if (NULL == pInode)
	{
		return;
	}
	this->Open1(pInode, u.u_arg[1], 0);
}

void FileManager::Creat()
{
	Inode* pInode;
	User& u = g_User;

	/* 这里不设置读写权限了，因为并没有对其进行检查 */
	unsigned int newACCMode = u.u_arg[1];

	/* 搜索目录的模式为1，表示创建；若父目录不可写，出错返回 */
	pInode = this->NameI(NextChar, FileManager::CREATE);

	/* 没有找到相应的Inode，或NameI出错 */
	if (NULL == pInode)
	{
		if (u.u_error)
			return;
		/* 创建Inode */
		pInode = this->MakNode(newACCMode);
		/* 创建失败 */
		if (NULL == pInode)
		{
			return;
		}

		/* 如果所希望的名字不存在，使用参数trf = 2来调用open1() */
		this->Open1(pInode, File::FWRITE, 2);
	}
	else
	{
		/* 如果NameI()搜索到已经存在要创建的文件，则清空该文件（用算法ITrunc()） */
		this->Open1(pInode, File::FWRITE, 1);
		pInode->i_mode |= newACCMode;
	}
}

void FileManager::Open1(Inode* pInode, int mode, int trf)
{
	User& u = g_User;

	if (u.u_error)
	{
		this->m_InodeTable->PutI(pInode);
		printf("Open1 函数中出错, u_error为: %d", int(u.u_error));
		return;
	}

	/* 在creat文件的时候搜索到同文件名的文件，释放该文件所占据的所有盘块 */
	if (1 == trf)
	{
		pInode->TruncI();
	}

	/* 分配打开文件控制块File结构 */
	File* pFile = this->m_OpenFileTable->AllocF();
	if (NULL == pFile)
	{
		this->m_InodeTable->PutI(pInode);
		return;
	}
	/* 设置打开文件方式，建立File结构和内存Inode的勾连关系 */
	pFile->f_flag = mode & (File::FREAD | File::FWRITE);
	pFile->f_inode = pInode;

	/* 为打开或者创建文件的各种资源都已成功分配，函数返回 */
	if (u.u_error == 0)
	{
		return;
	}
	else	/* 如果出错则释放资源 */
	{
		/* 释放打开文件描述符 */
		int fd = u.u_ar0[User::EAX];
		if (fd != -1)
		{
			u.u_ofiles.SetF(fd, NULL);
			/* 递减File结构和Inode的引用计数 ,File结构没有锁 f_count为0就是释放File结构了*/
			pFile->f_count--;
		}
		this->m_InodeTable->PutI(pInode);
	}
}

void FileManager::Close()
{
	User& u = g_User;
	int fd = u.u_arg[0];

	/* 获取打开文件控制块File结构 */
	File* pFile = u.u_ofiles.GetF(fd);
	if (NULL == pFile)
	{
		return;
	}

	/* 释放打开文件描述符fd，递减File结构引用计数 */
	u.u_ofiles.SetF(fd, NULL);
	this->m_OpenFileTable->CloseF(pFile);
}

void FileManager::Seek()
{
	File* pFile;
	User& u = g_User;
	int fd = u.u_arg[0];

	pFile = u.u_ofiles.GetF(fd);
	if (NULL == pFile)
	{
		return;  /* 若FILE不存在，GetF有设出错码 */
	}

	int offset = u.u_arg[1];

	/* 如果u.u_arg[2]在3 ~ 5之间，那么长度单位由字节变为512字节 */
	if (u.u_arg[2] > 2)
	{
		offset = offset << 9;
		u.u_arg[2] -= 3;
	}

	switch (u.u_arg[2])
	{
		/* 读写位置设置为offset */
	case 0:
		pFile->f_offset = offset;
		break;
		/* 读写位置加offset(可正可负) */
	case 1:
		pFile->f_offset += offset;
		break;
		/* 读写位置调整为文件长度加offset */
	case 2:
		pFile->f_offset = pFile->f_inode->i_size + offset;
		break;
	}
}

void FileManager::Read()
{
	/* 直接调用Rdwr()函数即可 */
	this->Rdwr(File::FREAD);
}

void FileManager::Write()
{
	/* 直接调用Rdwr()函数即可 */
	this->Rdwr(File::FWRITE);
}

void FileManager::Rdwr(enum File::FileFlags mode)
{
	File* pFile;
	User& u = g_User;

	/* 根据Read()/Write()的系统调用参数fd获取打开文件控制块结构 */
	pFile = u.u_ofiles.GetF(u.u_arg[0]);	/* fd */
	if (NULL == pFile)
	{
		/* 不存在该打开文件，GetF已经设置过出错码，所以这里不需要再设置了 */
		/*	u.u_error = User::EBADF;	*/
		return;
	}


	/* 读写的模式不正确 */
	if ((pFile->f_flag & mode) == 0)
	{
		u.u_error = User::U_EACCES;
		return;
	}

	u.u_IOParam.m_Base = (unsigned char*)u.u_arg[1];	/* 目标缓冲区首址 */
	u.u_IOParam.m_Count = u.u_arg[2];		/* 要求读/写的字节数 */

	/* 设置文件起始读位置 */
	u.u_IOParam.m_Offset = pFile->f_offset;
	if (File::FREAD == mode)
	{
		pFile->f_inode->ReadI();
	}
	else
	{
		pFile->f_inode->WriteI();
	}

	/* 根据读写字数，移动文件读写偏移指针 */
	pFile->f_offset += (u.u_arg[2] - u.u_IOParam.m_Count);

	/* 返回实际读写的字节数，修改存放系统调用返回值的核心栈单元 */
	u.u_ar0[User::EAX] = u.u_arg[2] - u.u_IOParam.m_Count;
}

Inode* FileManager::NameI(char (*func)(), enum DirectorySearchMode mode)
{
	Inode* pInode;
	Buf* pBuf;
	char curchar;
	char* pChar;
	int freeEntryOffset;	/* 以创建文件模式搜索目录时，记录空闲目录的偏移量 */
	User& u = g_User;
	BufferManager& bufferManager = g_BufferManager;

	/* 如果路径是'/'开头的，则从根目录开始搜索，否则从当前进程 的工作目录开始搜索 */
	if ('/' == (curchar = (*func)())) {
		pInode = this->rootDirInode;
	}
	else {
		pInode = u.u_cdir;
	}
	/* 当前的寻找目录，加载到InodeTable中 */
	m_InodeTable->GetI(pInode->i_number);

	/* 允许出现/////a//b这种路径 */
	while ('/' == curchar) {
		curchar = (*func)();
	}
	/* 如果试图更改和删除当前目录文件则出错（经过上述循环，如果直接到达了路径的结尾） */
	if ('\0' == curchar && mode != FileManager::OPEN)
	{
		u.u_error = User::U_ENOENT;
		goto out;
	}

	/* 外层循环每次处理pathname中一段路径分量 */
	while (true) {
		/* 如果出错，则释放当前搜索的目录文件的Inode */
		if (u.u_error != User::U_NOERROR) {
			break;
		}
		if ('\0' == curchar) {
			return pInode;
		}
		/* 如果要进行搜索的不是目录文件，则释放资源并退出 */
		if ((pInode->i_mode & Inode::IFMT) != Inode::IFDIR)
		{
			u.u_error = User::U_ENOTDIR;
			break;	/* goto out; */
		}

		/*
		 * 将Pathname中当前准备进行匹配的路径分量拷贝到u.u_dbuf[]中，
		 * 便于和目录项进行比较。
		 */
		pChar = &(u.u_dbuf[0]);
		while ('/' != curchar && '\0' != curchar && u.u_error == User::U_NOERROR)
		{
			if (pChar < &(u.u_dbuf[DirectoryEntry::DIRSIZ]))
			{
				*pChar = curchar;
				pChar++;
			}
			curchar = (*func)();
		}
		/* 将u_dbuf剩余的部分填充为'\0' */
		while (pChar < &(u.u_dbuf[DirectoryEntry::DIRSIZ]))
		{
			*pChar = '\0';
			pChar++;
		}

		while ('/' == curchar)
		{
			curchar = (*func)();
		}
		if (u.u_error != User::U_NOERROR)
		{
			break; /* goto out; */
		}

		/* 内层循环部分对于u.u_dbuf[]中的路径名分量，逐个搜寻匹配的目录项 */
		u.u_IOParam.m_Offset = 0;
		/* 设置为目录项个数 ，含空白的目录项*/
		u.u_IOParam.m_Count = pInode->i_size / (DirectoryEntry::DIRSIZ + 4);
		freeEntryOffset = 0;
		pBuf = NULL;
		/* 内层循环，搜索当前Inode块中的目录项，是否有匹配的 */
		while (true) {
			/* 对目录项已经搜索完毕 */
			if (u.u_IOParam.m_Count == 0) {
				if (NULL != pBuf) {
					bufferManager.RelseB(pBuf);
				}
				/* 如果是创建新文件 */
				if (FileManager::CREATE == mode && curchar == '\0') {
					/* 将父目录Inode指针保存起来，以后写目录项WriteDir()函数会用到 */
					u.u_pdir = pInode;

					if (freeEntryOffset)	/* 此变量存放了空闲目录项位于目录文件中的偏移量 */
					{
						/* 将空闲目录项偏移量存入u区中，写目录项WriteDir()会用到 */
						/* 解释一下长度：DIRSIZ + sizeof(int) = sizeof(DirectoryEntry) */
						u.u_IOParam.m_Offset = freeEntryOffset - (DirectoryEntry::DIRSIZ + 4);
					}
					else  /*问题：为何if分支没有置IUPD标志？  这是因为文件的长度没有变呀*/
					{
						pInode->i_flag |= Inode::IUPD;
					}
					/* 找到可以写入的空闲目录项位置，NameI()函数返回 */
					return NULL;
				}
				/* 目录项搜索完毕而没有找到匹配项，释放资源并退出 */
				u.u_error = User::U_ENOENT;
				goto out;
			}
			
			/* 已经读完目录文件的当前盘块，需要读入下一目录项数据块 */
			if (u.u_IOParam.m_Offset % Inode::BLOCK_SIZE == 0) {
				/* 对于缓存块，找一个新的缓存 */
				if (pBuf != NULL) {
					bufferManager.RelseB(pBuf);
				}
				/* 计算要读的物理盘块号 */
				int phyBlkno = pInode->Bmap(u.u_IOParam.m_Offset / Inode::BLOCK_SIZE);
				pBuf = bufferManager.ReadB(phyBlkno);
			}

			/* 没有读完当前目录块，则读取下一目录项到u_dent */
			int* src = (int*)(pBuf->b_addr + (u.u_IOParam.m_Offset % Inode::BLOCK_SIZE));
			Utility::DWordCopy(src, (int*)&u.u_dent, sizeof(DirectoryEntry) / sizeof(int));

			u.u_IOParam.m_Offset += (DirectoryEntry::DIRSIZ + 4);
			u.u_IOParam.m_Count--;

			/* 如果是空闲目录项，记录该项位于目录文件中偏移量 */
			if (0 == u.u_dent.m_ino) {
				if (0 == freeEntryOffset) {
					freeEntryOffset = u.u_IOParam.m_Offset;
				}
				continue;
			}

			int i;
			for (i = 0; i < DirectoryEntry::DIRSIZ; ++i) {
				if (u.u_dbuf[i] != u.u_dent.m_name[i]) {
					break; /* 匹配至某一字符不相同，跳出for循环 */
				}
			}

			if (i < DirectoryEntry::DIRSIZ) {
				/* 不是要搜索的目录项，继续匹配下一目录项 */
				continue;
			}
			else {
				/* 目录项匹配成功，回到外层While(true)循环 */
				break;
			}
		}

		/*
		 * 从内层目录项匹配循环跳至此处，说明pathname中
		 * 当前路径分量匹配成功了，还需匹配pathname中下一路径
		 * 分量，直至遇到'\0'结束。
		 */
		if (NULL != pBuf)
		{
			bufferManager.RelseB(pBuf);
		}

		/* 如果是删除操作，则返回父目录Inode，而要删除文件的Inode号在u.u_dent.m_ino中 */
		if (FileManager::DELETE == mode && '\0' == curchar)
		{
			return pInode;
		}

		/*
		 * 匹配目录项成功，则释放当前目录Inode，根据匹配成功的
		 * 目录项m_ino字段获取相应下一级目录或文件的Inode。
		 */
		this->m_InodeTable->PutI(pInode);
		pInode = this->m_InodeTable->GetI(u.u_dent.m_ino);
		/* 回到外层While(true)循环，继续匹配Pathname中下一路径分量 */

		if (NULL == pInode)	/* 获取失败 */
		{
			return NULL;
		}
	}
out:
	this->m_InodeTable->PutI(pInode);
	return NULL;
}

char FileManager::NextChar()
{
	User& u = g_User;

	/* u.u_dirp指向pathname中的字符 */
	return *u.u_dirp++;
}

Inode* FileManager::MakNode(unsigned int mode)
{
	Inode* pInode;
	User& u = g_User;

	/* 分配一个空闲DiskInode，里面内容已全部清空 */
	pInode = this->m_FileSystem->AllocI();
	if (NULL == pInode)
	{
		return NULL;
	}

	pInode->i_flag |= (Inode::IACC | Inode::IUPD);
	pInode->i_mode = mode | Inode::IALLOC;
	pInode->i_nlink = 1;

	/* 将目录项写入u.u_dent，随后写入目录文件 */
	this->WriteDir(pInode);
	return pInode;
}

void FileManager::WriteDir(Inode* pInode)
{
	User& u = g_User;

	/* 设置目录项中Inode编号部分 */
	u.u_dent.m_ino = pInode->i_number;

	/* 设置目录项中pathname分量部分 */
	for (int i = 0; i < DirectoryEntry::DIRSIZ; i++)
	{
		u.u_dent.m_name[i] = u.u_dbuf[i];
	}

	u.u_IOParam.m_Count = DirectoryEntry::DIRSIZ + 4;
	u.u_IOParam.m_Base = (unsigned char*)&u.u_dent;

	/* 将目录项写入父目录文件 */
	u.u_pdir->WriteI();
	this->m_InodeTable->PutI(u.u_pdir);
}

void FileManager::SetCurDir(char* pathname)
{
	User& u = g_User;

	/* 路径不是从根目录'/'开始，则在现有u.u_curdir后面加上当前路径分量 */
	if (pathname[0] != '/')
	{
		int length = Utility::StringLength(u.u_curdir);
		if (u.u_curdir[length - 1] != '/')
		{
			u.u_curdir[length] = '/';
			length++;
		}
		Utility::StringCopy(pathname, u.u_curdir + length);
	}
	else	/* 如果是从根目录'/'开始，则取代原有工作目录 */
	{
		Utility::StringCopy(pathname, u.u_curdir);
	}
}

void FileManager::ChDir()
{
	Inode* pInode;
	User& u = g_User;

	pInode = this->NameI(FileManager::NextChar, FileManager::OPEN);
	if (NULL == pInode)
	{
		return;
	}
	/* 搜索到的文件不是目录文件 */
	if ((pInode->i_mode & Inode::IFMT) != Inode::IFDIR)
	{
		u.u_error = User::U_ENOTDIR;
		this->m_InodeTable->PutI(pInode);
		return;
	}

	/* 将之前打开的目录Inode关闭 */
	this->m_InodeTable->PutI(u.u_cdir);
	/* 切换到当前目录的Inode */
	u.u_cdir = pInode;

	this->SetCurDir((char*)u.u_arg[0] /* pathname */);
}

void FileManager::UnLink()
{
	Inode* pInode;
	Inode* pDeleteInode;
	User& u = g_User;

	pDeleteInode = this->NameI(FileManager::NextChar, FileManager::DELETE);
	if (NULL == pDeleteInode)
	{
		return;
	}

	pInode = this->m_InodeTable->GetI(u.u_dent.m_ino);
	if (NULL == pInode)
	{
		printf("unlink -- geti");
	}

	/* 写入清零后的目录项 */
	u.u_IOParam.m_Offset -= (DirectoryEntry::DIRSIZ + 4);
	u.u_IOParam.m_Base = (unsigned char*)&u.u_dent;
	u.u_IOParam.m_Count = DirectoryEntry::DIRSIZ + 4;

	u.u_dent.m_ino = 0;
	pDeleteInode->WriteI();

	/* 修改inode项 */
	pInode->i_nlink--;
	pInode->i_flag |= Inode::IUPD;

	this->m_InodeTable->PutI(pDeleteInode);
	this->m_InodeTable->PutI(pInode);
}

/*==========================class DirectoryEntry===============================*/
DirectoryEntry::DirectoryEntry()
{
	this->m_ino = 0;
	this->m_name[0] = '\0';
}

DirectoryEntry::~DirectoryEntry()
{
	//nothing to do here
}
