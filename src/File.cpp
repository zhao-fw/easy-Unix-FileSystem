#include "File.h"
#include "Utility.h"
#include "User.h"

/*
 * ���ڲ���Ҫʵ��kernel�����ݣ����Զ�����kernel�ж���� g_BufferManager, g_FileSystem, g_user�����ݣ�
 * ֱ����main�ж����ȫ�ֱ�����ʵ�֡�
 */
extern User g_User;

/*==============================class File===================================*/
File::File()
{
	this->f_count = 0;
	this->f_flag = 0;
	this->f_offset = 0;
	this->f_inode = NULL;
}

File::~File()
{
	//nothing to do here
}

/*==============================class OpenFiles===================================*/
OpenFiles::OpenFiles()
{
}

OpenFiles::~OpenFiles()
{
}

int OpenFiles::AllocFreeSlot()
{
	int i;
	User& u = g_User;

	/* ע��c�в�������ѭ���ж��������C++�п��� */
	for (i = 0; i < OpenFiles::NOFILES; i++)
	{
		/* ���̴��ļ������������ҵ�������򷵻�֮ */
		if (this->ProcessOpenFileTable[i] == NULL)
		{
			/* ���ú���ջ�ֳ��������е�EAX�Ĵ�����ֵ����ϵͳ���÷���ֵ */
			u.u_ar0[User::EAX] = i;
			return i;
		}
	}

	u.u_ar0[User::EAX] = -1;   /* Open1����Ҫһ����־�������ļ��ṹ����ʧ��ʱ�����Ի���ϵͳ��Դ*/
	u.u_error = User::U_EMFILE;
	return -1;
}

File* OpenFiles::GetF(int fd)
{
	File* pFile;
	User& u = g_User;

	/* ������ļ���������ֵ�����˷�Χ */
	if (fd < 0 || fd >= OpenFiles::NOFILES)
	{
		u.u_error = User::U_EBADF;
		return NULL;
	}

	pFile = this->ProcessOpenFileTable[fd];
	if (pFile == NULL)
	{
		u.u_error = User::U_EBADF;
	}

	return pFile;	/* ��ʹpFile==NULLҲ���������ɵ���GetF�ĺ������жϷ���ֵ */
}

void OpenFiles::SetF(int fd, File* pFile)
{
	if (fd < 0 || fd >= OpenFiles::NOFILES)
	{
		return;
	}
	/* ���̴��ļ�������ָ��ϵͳ���ļ�������Ӧ��File�ṹ */
	this->ProcessOpenFileTable[fd] = pFile;
}

/*==============================class IOParameter===================================*/
IOParameter::IOParameter()
{
	this->m_Base = 0;
	this->m_Count = 0;
	this->m_Offset = 0;
}

IOParameter::~IOParameter()
{
	//nothing to do here
}

