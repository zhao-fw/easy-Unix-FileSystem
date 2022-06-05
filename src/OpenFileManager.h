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
 * ���ļ�������(OpenFileManager)�����ں��жԴ��ļ������Ĺ���Ϊ���̴��ļ������ں����ݽṹ֮��Ĺ�����ϵ��
 * 
 * ������ϵָ����u���д��ļ�������ָ����ļ����е�File���ļ����ƽṹ���Լ���File�ṹָ���ļ���Ӧ���ڴ�Inode��
 */
class OpenFileTable
{
public:
	static const int NFILE = 100;	/* ���ļ����ƿ�File�ṹ������ */

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
	 * @comment ��ʽ��OpenFileTable
	*/
	void Format();

public:
	File m_File[NFILE];		/* ϵͳ���ļ���Ϊ���н��̹������̴��ļ����������а���ָ����ļ����ж�ӦFile�ṹ��ָ�롣*/
};

/*
 * �ڴ�Inode��(class InodeTable)
 * �����ڴ�Inode�ķ�����ͷš�
 */
class InodeTable
{
public:
	static const int NINODE = 100;	/* �ڴ�Inode������ */

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
	 * @comment ����ָ�����Inode��Ż�ȡ��Ӧ
	 * Inode�������Inode�Ѿ����ڴ��У����ظ��ڴ�Inode����������ڣ�����NULL
	 */
	Inode* GetI(int inumber);

	/*
	 * @comment ���ٸ��ڴ�Inode�����ü����������Inode�Ѿ�û��Ŀ¼��ָ���������޽������ø�Inode�����ͷŴ��ļ�ռ�õĴ��̿顣
	 * 
	 *	close�ļ�ʱ�����Iput
	 *	��Ҫ���Ĳ������ڴ�i�ڵ���� i_count--����Ϊ0���ͷ��ڴ� i�ڵ㡢���иĶ�д�ش���
	 *		�����ļ�;��������Ŀ¼�ļ������������󶼻�PutI���ڴ�i�ڵ㡣·�����ĵ�����2��·������һ���Ǹ�
	 *		Ŀ¼�ļ�������������д������ļ�������ɾ��һ�������ļ���������������д���ɾ����Ŀ¼����ô
	 *   	���뽫���Ŀ¼�ļ�����Ӧ���ڴ� i�ڵ�д�ش��̡�
	 *   	���Ŀ¼�ļ������Ƿ��������ģ����Ǳ��뽫����i�ڵ�д�ش��̡�
	 */
	void PutI(Inode* pNode);

	/*
	 * @comment �����б��޸Ĺ����ڴ�Inode���µ���Ӧ���Inode��
	 */
	void UpdateInodeTable();

	/*
	 * @comment �����Ϊinumber�����inode�Ƿ����ڴ濽����
	 * ������򷵻ظ��ڴ�Inode���ڴ�Inode���е�����
	 */
	int IsLoaded(int inumber);

	/*
	 * @comment ���ڴ�Inode����Ѱ��һ�����е��ڴ�Inode
	 */
	Inode* GetFreeInode();

	/*
	 * @commet ��ʽ��InodeTable
	*/
	void Format();

public:
	Inode m_Inode[NINODE];		/* �ڴ�Inode���飬ÿ�����ļ�����ռ��һ���ڴ�Inode */
	FileSystem* m_FileSystem;	/* ��ȫ�ֶ���g_FileSystem������ */
};

#endif
