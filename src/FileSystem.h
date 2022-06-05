#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "INode.h"
#include "Buf.h"
#include "BufferManager.h"

/*
 * �ļ�ϵͳ�洢��Դ�����(Super Block)�Ķ��塣
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
	int		s_isize;		/* ���Inode��ռ�õ��̿��� */
	int		s_fsize;		/* �̿����� */

	int		s_nfree;		/* ֱ�ӹ���Ŀ����̿����� */
	int		s_free[100];	/* ֱ�ӹ���Ŀ����̿������� */

	int		s_ninode;		/* ֱ�ӹ���Ŀ������Inode���� */
	int		s_inode[100];	/* ֱ�ӹ���Ŀ������Inode������ */

	int		s_flock;		/* ���������̿��������־ */
	int		s_ilock;		/* ��������Inode���־ */

	int		s_fmod;			/* �ڴ���super block�������޸ı�־����ζ����Ҫ��������Ӧ��Super Block */
	int		s_ronly;		/* ���ļ�ϵͳֻ�ܶ��� */
	int		s_time;			/* ���һ�θ���ʱ�� */
	int		padding[47];	/* ���ʹSuperBlock���С����1024�ֽڣ�ռ��2������ */
};

/*
 * �ļ�ϵͳ��(FileSystem)�����ļ��洢�豸��
 * �ĸ���洢��Դ�����̿顢���INode�ķ��䡢
 * �ͷš�
 */
class FileSystem
{
public:
	/* static consts */

	/* ���������������� */
	static const int SECTOR_NUM = 18000;
	static const int BLOCK_NUM = 18000;

	/* 
	 * ������Block�Ĵ�С������򵥴���ѡȡ�Ĵ�Сͬ�����Ĵ�С
	 * ����һ�����ϵͳ�У��޷�������ֱ�ӽ�����������ͨ������ʵ�ֵģ�һ����һ��Ϊ4k��С��
	*/
	static const int BLOCK_SIZE = 512;

	/* �ļ�ϵͳ��Ŀ¼���Inode��� */
	static const int ROOTINO = 0;

	/* ����SuperBlock��س��� */
	static const int SUPER_BLOCK_START_SECTOR = 0;		/* SuperBlockλ�ڴ����ϵ���ʼ�����ţ�ռ������������0��1���� */
	static const int SUPER_BLOCK_SIZE = 2;

	/* ������Inode��س��� */
	static const int INODE_NUMBER_PER_SECTOR = 8;		/* ���INode���󳤶�Ϊ64�ֽڣ�ÿ�����̿���Դ��512/64 = 8�����Inode */
	static const int INODE_ZONE_START_SECTOR = 2;		/* ���Inode��λ�ڴ����ϵ���ʼ������ */
	static const int INODE_ZONE_SIZE = 1024 - 2;		/* ���������Inode��ռ�ݵ������� */

	/* ����������������س��� */
	static const int DATA_ZONE_START_SECTOR = 1024;		/* ����������ʼ������ */
	static const int DATA_ZONE_END_SECTOR = 18000 - 1;	/* �������Ľ��������� */
	static const int DATA_ZONE_SIZE = 18000 - DATA_ZONE_START_SECTOR;	/* ������ռ�ݵ��������� */

	/* Functions */
public:
	/* Constructors */
	FileSystem();
	/* Destructors */
	~FileSystem();

	/*
	 * @comment	��ʽ��SuperBlock
	*/
	void FormatSuperBlock();

	/*
	 * @comment	��ʽ�������ļ�
	*/
	void FormatDevice();

	/*
	 * @comment ��ʼ����Ա����
	 */
	void Initialize();

	/*
	* @comment ϵͳ��ʼ��ʱ����SuperBlock
	*/
	void LoadSuperBlock();

	/*
	 * @comment ��SuperBlock������ڴ渱�����µ�
	 * �洢�豸��SuperBlock��ȥ
	 */
	void Update();

	/*
	 * @comment  �ڴ洢�豸�Ϸ���һ������
	 * ���INode��һ�����ڴ����µ��ļ���
	 */
	Inode* AllocI();
	/*
	 * @comment  �ͷŴ洢�豸dev�ϱ��Ϊnumber
	 * �����INode��һ������ɾ���ļ���
	 */
	void FreeI(int number);

	/*
	 * @comment �ڻ����з�����л�����
	 */
	Buf* AllocBlock();
	/*
	 * @comment �ͷű��Ϊblkno�Ĵ��̿�
	 */
	void FreeBlock(int blkno);

private:
	/*
	 * @comment ����豸dev�ϱ��blkno�Ĵ��̿��Ƿ�����
	 * �����̿���
	 */
	bool BadBlock(SuperBlock* spb, int blkno);

	/* Members */
public:
	Device* m_Device;					/* ָ��ʵ�ʵĴ����ļ� */
	BufferManager* m_BufferManager;		/* FileSystem����Ҫ�������ģ��(BufferManager)�ṩ�Ľӿ� */
	SuperBlock* m_SuperBlock;			/* ָ���ļ�ϵͳ��Super Block�������ڴ��еĸ��� */
};

#endif
