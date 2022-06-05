#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "INode.h"
#include "Buf.h"
#include "BufferManager.h"

/*
 * 文件系统存储资源管理块(Super Block)的定义。
 */
class SuperBlock
{
	/* Functions */
public:
	/* Constructors */
	SuperBlock();
	/* Destructors */
	~SuperBlock();

	/* Members */
public:
	int		s_isize;		/* 外存Inode区占用的盘块数 */
	int		s_fsize;		/* 盘块总数 */

	int		s_nfree;		/* 直接管理的空闲盘块数量 */
	int		s_free[100];	/* 直接管理的空闲盘块索引表 */

	int		s_ninode;		/* 直接管理的空闲外存Inode数量 */
	int		s_inode[100];	/* 直接管理的空闲外存Inode索引表 */

	int		s_flock;		/* 封锁空闲盘块索引表标志 */
	int		s_ilock;		/* 封锁空闲Inode表标志 */

	int		s_fmod;			/* 内存中super block副本被修改标志，意味着需要更新外存对应的Super Block */
	int		s_ronly;		/* 本文件系统只能读出 */
	int		s_time;			/* 最近一次更新时间 */
	int		padding[47];	/* 填充使SuperBlock块大小等于1024字节，占据2个扇区 */
};

/*
 * 文件系统类(FileSystem)管理文件存储设备中
 * 的各类存储资源，磁盘块、外存INode的分配、
 * 释放。
 */
class FileSystem
{
public:
	/* static consts */

	/* 定义了扇区的数量 */
	static const int SECTOR_NUM = 18000;
	static const int BLOCK_NUM = 18000;

	/* 
	 * 定义了Block的大小，这里简单处理，选取的大小同扇区的大小
	 * （在一般操作系统中，无法对扇区直接进行索引，是通过块来实现的，一个块一般为4k大小）
	*/
	static const int BLOCK_SIZE = 512;

	/* 文件系统根目录外存Inode编号 */
	static const int ROOTINO = 0;

	/* 定义SuperBlock相关常量 */
	static const int SUPER_BLOCK_START_SECTOR = 0;		/* SuperBlock位于磁盘上的起始扇区号，占据两个扇区，0，1扇区 */
	static const int SUPER_BLOCK_SIZE = 2;

	/* 定义了Inode相关常量 */
	static const int INODE_NUMBER_PER_SECTOR = 8;		/* 外存INode对象长度为64字节，每个磁盘块可以存放512/64 = 8个外存Inode */
	static const int INODE_ZONE_START_SECTOR = 2;		/* 外存Inode区位于磁盘上的起始扇区号 */
	static const int INODE_ZONE_SIZE = 1024 - 2;		/* 磁盘上外存Inode区占据的扇区数 */

	/* 定义了数据区的相关常量 */
	static const int DATA_ZONE_START_SECTOR = 1024;		/* 数据区的起始扇区号 */
	static const int DATA_ZONE_END_SECTOR = 18000 - 1;	/* 数据区的结束扇区号 */
	static const int DATA_ZONE_SIZE = 18000 - DATA_ZONE_START_SECTOR;	/* 数据区占据的扇区数量 */

	/* Functions */
public:
	/* Constructors */
	FileSystem();
	/* Destructors */
	~FileSystem();

	/*
	 * @comment	格式化SuperBlock
	*/
	void FormatSuperBlock();

	/*
	 * @comment	格式化磁盘文件
	*/
	void FormatDevice();

	/*
	 * @comment 初始化成员变量
	 */
	void Initialize();

	/*
	* @comment 系统初始化时读入SuperBlock
	*/
	void LoadSuperBlock();

	/*
	 * @comment 将SuperBlock对象的内存副本更新到
	 * 存储设备的SuperBlock中去
	 */
	void Update();

	/*
	 * @comment  在存储设备上分配一个空闲
	 * 外存INode，一般用于创建新的文件。
	 */
	Inode* AllocI();
	/*
	 * @comment  释放存储设备dev上编号为number
	 * 的外存INode，一般用于删除文件。
	 */
	void FreeI(int number);

	/*
	 * @comment 在缓存中分配空闲缓存区
	 */
	Buf* AllocBlock();
	/*
	 * @comment 释放编号为blkno的磁盘块
	 */
	void FreeBlock(int blkno);

private:
	/*
	 * @comment 检查设备dev上编号blkno的磁盘块是否属于
	 * 数据盘块区
	 */
	bool BadBlock(SuperBlock* spb, int blkno);

	/* Members */
public:
	Device* m_Device;					/* 指向实际的磁盘文件 */
	BufferManager* m_BufferManager;		/* FileSystem类需要缓存管理模块(BufferManager)提供的接口 */
	SuperBlock* m_SuperBlock;			/* 指向文件系统的Super Block对象在内存中的副本 */
};

#endif
