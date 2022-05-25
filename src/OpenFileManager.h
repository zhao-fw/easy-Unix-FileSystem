#ifndef OPEN_FILE_MANAGER_H
#define OPEN_FILE_MANAGER_H

#include "INode.h"
#include "File.h"
#include "FileSystem.h"

/* Forward Declaration */
class OpenFileTable;
class InodeTable;

/* ����2������ʵ��������OpenFileManager.cpp�ļ��� */
extern InodeTable g_InodeTable;
extern OpenFileTable g_OpenFileTable;

/*
 * ���ļ�������(OpenFileManager)����
 * �ں��жԴ��ļ������Ĺ���Ϊ����
 * ���ļ������ں����ݽṹ֮��Ĺ���
 * ��ϵ��
 * ������ϵָ����u���д��ļ�������ָ��
 * ���ļ����е�File���ļ����ƽṹ��
 * �Լ���File�ṹָ���ļ���Ӧ���ڴ�Inode��
 */
class OpenFileTable
{
public:
	/* static consts */
	static const int NFILE = 100;	/* ���ļ����ƿ�File�ṹ������ */

	/* Functions */
public:
	/* Constructors */
	OpenFileTable();
	/* Destructors */
	~OpenFileTable();

	/*
	 * @comment ��ϵͳ���ļ����з���һ�����е�File�ṹ
	 */
	File* AllocF();
	/*
	 * @comment �Դ��ļ����ƿ�File�ṹ�����ü���f_count��1��
	 * �����ü���f_countΪ0�����ͷ�File�ṹ��
	 */
	void CloseF(File* pFile);

	/*
	 * @comment �����е��ļ�������ա�
	 * ѧ����
	*/
	void Format();

	/* Members */
public:
	File m_File[NFILE];			/* ϵͳ���ļ���Ϊ���н��̹������̴��ļ���������
								�а���ָ����ļ����ж�ӦFile�ṹ��ָ�롣*/
};

/*
 * �ڴ�Inode��(class InodeTable)
 * �����ڴ�Inode�ķ�����ͷš�
 */
class InodeTable
{
	/* static consts */
public:
	static const int NINODE = 100;	/* �ڴ�Inode������ */

	/* Functions */
public:
	/* Constructors */
	InodeTable();
	/* Destructors */
	~InodeTable();

	/*
	 * @comment ��ʼ����g_FileSystem���������
	 */
	void Initialize();
	/*
	 * @comment ����ָ���豸��dev�����Inode��Ż�ȡ��Ӧ
	 * Inode�������Inode�Ѿ����ڴ��У��������������ظ��ڴ�Inode��
	 * ��������ڴ��У���������ڴ�����������ظ��ڴ�Inode
	 */
	Inode* GetI(short dev, int inumber);
	/*
	 * @comment ���ٸ��ڴ�Inode�����ü����������Inode�Ѿ�û��Ŀ¼��ָ������
	 * ���޽������ø�Inode�����ͷŴ��ļ�ռ�õĴ��̿顣
	 */
	void PutI(Inode* pNode);

	/*
	 * @comment �����б��޸Ĺ����ڴ�Inode���µ���Ӧ���Inode��
	 */
	void UpdateInodeTable();

	/*
	 * @comment ����豸dev�ϱ��Ϊinumber�����inode�Ƿ����ڴ濽����
	 * ������򷵻ظ��ڴ�Inode���ڴ�Inode���е�����
	 */
	int IsLoaded(short dev, int inumber);
	/*
	 * @comment ���ڴ�Inode����Ѱ��һ�����е��ڴ�Inode
	 */
	Inode* GetFreeInode();

	/*
	 * @commet �����е�Inode������ա�
	 * ѧ����
	*/
	/* Members */
public:
	Inode m_Inode[NINODE];		/* �ڴ�Inode���飬ÿ�����ļ�����ռ��һ���ڴ�Inode */

	FileSystem* m_FileSystem;	/* ��ȫ�ֶ���g_FileSystem������ */
};

#endif
