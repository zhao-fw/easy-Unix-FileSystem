#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "FileSystem.h"
#include "OpenFileManager.h"
#include "File.h"


/*
 * �ļ�������(FileManager)
 * ��װ���ļ�ϵͳ�ĸ���ϵͳ�����ں���̬�´�����̣�
 * ����ļ���Open()��Close()��Read()��Write()�ȵ�
 * ��װ�˶��ļ�ϵͳ���ʵľ���ϸ�ڡ�
 */
class FileManager
{
public:
	/* Ŀ¼����ģʽ������NameI()���� */
	enum DirectorySearchMode
	{
		OPEN = 0,		/* �Դ��ļ���ʽ����Ŀ¼ */
		CREATE = 1,		/* ���½��ļ���ʽ����Ŀ¼ */
		DELETE = 2		/* ��ɾ���ļ���ʽ����Ŀ¼ */
	};

	/* Functions */
public:
	/* Constructors */
	FileManager();
	/* Destructors */
	~FileManager();


	/*
	 * @comment ��ʼ����ȫ�ֶ��������
	 */
	void Initialize();

	/*
	 * @comment Open()ϵͳ���ô������
	 *  ���ܣ����ļ�
	 *  Ч�����������ļ��ṹ��i_count Ϊ������i_count ++��
	 */
	void Open();

	/*
	 * @comment Creat()ϵͳ���ô������
	 *  ���ܣ�����һ���µ��ļ�
	 *	Ч�����������ļ��ṹ���ڴ�i�ڵ㿪�� ��i_count Ϊ������Ӧ���� 1��
	 */
	void Creat();

	/*
	 * @comment Open()��Creat()ϵͳ���õĹ�������
	 *   trf == 0��open����
	 *   trf == 1��creat���ã�creat�ļ���ʱ��������ͬ�ļ������ļ�
	 *   trf == 2��creat���ã�creat�ļ���ʱ��δ������ͬ�ļ������ļ��������ļ�����ʱ��һ������
	 *   mode���������ļ�ģʽ����ʾ�ļ������� ����д���Ƕ�д

	 */
	void Open1(Inode* pInode, int mode, int trf);

	/*
	 * @comment Close()ϵͳ���ô������
	 */
	void Close();

	/*
	 * @comment Seek()ϵͳ���ô������
	 */
	void Seek();

	/*
	 * @comment Read()ϵͳ���ô������
	 */
	void Read();

	/*
	 * @comment Write()ϵͳ���ô������
	 */
	void Write();

	/*
	 * @comment ��дϵͳ���ù������ִ���
	 */
	void Rdwr(enum File::FileFlags mode);

	/*
	 * @comment Ŀ¼��������·��ת��Ϊ��Ӧ��Inode��
	 *	����NULL��ʾĿ¼����ʧ�ܣ������Ǹ�ָ�룬ָ���ļ����ڴ��i�ڵ�
	 */
	Inode* NameI(char (*func)(), enum DirectorySearchMode mode);

	/*
	 * @comment ��ȡ·���е���һ���ַ�
	 */
	static char NextChar();

	/*
	 * @comment ��Creat()ϵͳ����ʹ�ã�����Ϊ�������ļ������ں���Դ
	 * 
	 *	Ϊ�´������ļ�д�µ�i�ڵ���µ�Ŀ¼��
	 *	���ص�pInode�����������ڴ�i�ڵ㣬���е�i_count�� 1��
	 *	�ڳ����������� WriteDir��������������Լ���Ŀ¼��д����Ŀ¼���޸ĸ�Ŀ¼�ļ���i�ڵ� ������д�ش��̡�
	 */
	Inode* MakNode(unsigned int mode);

	/*
	 * @comment ��Ŀ¼��Ŀ¼�ļ�д��һ��Ŀ¼��
	 */
	void WriteDir(Inode* pInode);

	/*
	 * @comment ���õ�ǰ����·��
	 */
	void SetCurDir(char* pathname);

	/* 
	 * @comment �ı䵱ǰ����Ŀ¼
	 */
	void ChDir();

	/* 
	 * @comment ȡ���ļ�
	 */
	void UnLink();

public:
	/* ��Ŀ¼�ڴ�Inode */
	Inode* rootDirInode;

	/* ��ȫ�ֶ���g_FileSystem�����ã��ö���������ļ�ϵͳ�洢��Դ */
	FileSystem* m_FileSystem;

	/* ��ȫ�ֶ���g_InodeTable�����ã��ö������ڴ�Inode��Ĺ��� */
	InodeTable* m_InodeTable;

	/* ��ȫ�ֶ���g_OpenFileTable�����ã��ö�������ļ�����Ĺ��� */
	OpenFileTable* m_OpenFileTable;
};


class DirectoryEntry
{
	/* static members */
public:
	static const int DIRSIZ = 28;	/* Ŀ¼����·�����ֵ�����ַ������� */

	/* Functions */
public:
	/* Constructors */
	DirectoryEntry();
	/* Destructors */
	~DirectoryEntry();

	/* Members */
public:
	int m_ino;		/* Ŀ¼����Inode��Ų��� */
	char m_name[DIRSIZ];	/* Ŀ¼����·�������� */
};

#endif
