#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "FileSystem.h"
#include "OpenFileManager.h"
#include "File.h"


/*
 * 文件管理类(FileManager)
 * 封装了文件系统的各种系统调用在核心态下处理过程，
 * 如对文件的Open()、Close()、Read()、Write()等等
 * 封装了对文件系统访问的具体细节。
 */
class FileManager
{
public:
	/* 目录搜索模式，用于NameI()函数 */
	enum DirectorySearchMode
	{
		OPEN = 0,		/* 以打开文件方式搜索目录 */
		CREATE = 1,		/* 以新建文件方式搜索目录 */
		DELETE = 2		/* 以删除文件方式搜索目录 */
	};

	/* Functions */
public:
	/* Constructors */
	FileManager();
	/* Destructors */
	~FileManager();


	/*
	 * @comment 初始化对全局对象的引用
	 */
	void Initialize();

	/*
	 * @comment Open()系统调用处理过程
	 *  功能：打开文件
	 *  效果：建立打开文件结构，i_count 为正数（i_count ++）
	 */
	void Open();

	/*
	 * @comment Creat()系统调用处理过程
	 *  功能：创建一个新的文件
	 *	效果：建立打开文件结构，内存i节点开锁 、i_count 为正数（应该是 1）
	 */
	void Creat();

	/*
	 * @comment Open()、Creat()系统调用的公共部分
	 *   trf == 0由open调用
	 *   trf == 1由creat调用，creat文件的时候搜索到同文件名的文件
	 *   trf == 2由creat调用，creat文件的时候未搜索到同文件名的文件，这是文件创建时更一般的情况
	 *   mode参数：打开文件模式，表示文件操作是 读、写还是读写

	 */
	void Open1(Inode* pInode, int mode, int trf);

	/*
	 * @comment Close()系统调用处理过程
	 */
	void Close();

	/*
	 * @comment Seek()系统调用处理过程
	 */
	void Seek();

	/*
	 * @comment Read()系统调用处理过程
	 */
	void Read();

	/*
	 * @comment Write()系统调用处理过程
	 */
	void Write();

	/*
	 * @comment 读写系统调用公共部分代码
	 */
	void Rdwr(enum File::FileFlags mode);

	/*
	 * @comment 目录搜索，将路径转化为相应的Inode，
	 *	返回NULL表示目录搜索失败，否则是根指针，指向文件的内存打开i节点
	 */
	Inode* NameI(char (*func)(), enum DirectorySearchMode mode);

	/*
	 * @comment 获取路径中的下一个字符
	 */
	static char NextChar();

	/*
	 * @comment 被Creat()系统调用使用，用于为创建新文件分配内核资源
	 * 
	 *	为新创建的文件写新的i节点和新的目录项
	 *	返回的pInode是上了锁的内存i节点，其中的i_count是 1。
	 *	在程序的最后会调用 WriteDir，在这里把属于自己的目录项写进父目录，修改父目录文件的i节点 、将其写回磁盘。
	 */
	Inode* MakNode(unsigned int mode);

	/*
	 * @comment 向父目录的目录文件写入一个目录项
	 */
	void WriteDir(Inode* pInode);

	/*
	 * @comment 设置当前工作路径
	 */
	void SetCurDir(char* pathname);

	/* 
	 * @comment 改变当前工作目录
	 */
	void ChDir();

	/* 
	 * @comment 取消文件
	 */
	void UnLink();

public:
	/* 根目录内存Inode */
	Inode* rootDirInode;

	/* 对全局对象g_FileSystem的引用，该对象负责管理文件系统存储资源 */
	FileSystem* m_FileSystem;

	/* 对全局对象g_InodeTable的引用，该对象负责内存Inode表的管理 */
	InodeTable* m_InodeTable;

	/* 对全局对象g_OpenFileTable的引用，该对象负责打开文件表项的管理 */
	OpenFileTable* m_OpenFileTable;
};


class DirectoryEntry
{
	/* static members */
public:
	static const int DIRSIZ = 28;	/* 目录项中路径部分的最大字符串长度 */

	/* Functions */
public:
	/* Constructors */
	DirectoryEntry();
	/* Destructors */
	~DirectoryEntry();

	/* Members */
public:
	int m_ino;		/* 目录项中Inode编号部分 */
	char m_name[DIRSIZ];	/* 目录项中路径名部分 */
};

#endif
