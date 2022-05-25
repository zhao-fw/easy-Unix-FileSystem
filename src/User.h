#ifndef  USER_H
#define USER_H

#include "FileManager.h"
//#include <string>

class User {
public:
    static const int EAX = 0;	/* u.ar0[EAX]�������ֳ���������EAX�Ĵ�����ƫ���� */

    enum ErrorCode {
        NOERROR = 0,  	/* No u_error */
        EPERM = 1,	    /* Operation not permitted */
        ENOENT = 2,	    /* No such file or directory */
        //ESRCH = 3,	    /* No such process */
        //EINTR = 4,	    /* Interrupted system call */
        //EIO = 5,	    /* I/O u_error */
        //ENXIO = 6,	    /* No such device or address */
        //E2BIG = 7,	    /* Arg list too long */
        //ENOEXEC = 8,	/* Exec format u_error */
        EBADF = 9,	    /* Bad file number */
        //ECHILD = 10,	/* No child processes */
        //EAGAIN = 11,	/* Try again */
        //ENOMEM = 12,	/* Out of memory */
        EACCES = 13,	    /* Permission denied */
        //EFAULT = 14,	/* Bad address */
        //ENOTBLK = 15,	/* Block device required */
        //EBUSY = 16,	    /* Device or resource busy */
        //EEXIST = 17,	/* File exists */
        //EXDEV = 18,	    /* Cross-device link */
        //ENODEV = 19,	/* No such device */
        ENOTDIR = 20,	    /* Not a directory */
        //EISDIR = 21,	/* Is a directory */
        //EINVAL = 22,	/* Invalid argument */
        ENFILE = 23,	    /* File table overflow */
        EMFILE = 24,	    /* Too many open files */
        //ENOTTY = 25,	/* Not a typewriter(terminal) */
        //ETXTBSY = 26,	/* Text file busy */
        EFBIG = 27,	    /* File too large */
        ENOSPC = 28,	    /* No space left on device */
        //ESPIPE = 29,	/* Illegal seek */
        //EROFS = 30,	    /* Read-only file system */
        //EMLINK = 31,	/* Too many links */
        //EPIPE = 32,	    /* Broken pipe */
        //ENOSYS = 100,
        //EFAULT	= 106
    };

public:
    INode* u_cdir;		/* ָ��ǰĿ¼��Inodeָ�� */
    INode* u_pdir;		/* ָ��Ŀ¼��Inodeָ�� */

    DirectoryEntry u_dent;					/* ��ǰĿ¼��Ŀ¼�� */
    char u_dbuf[DirectoryEntry::DIRSIZ];	    /* ��ǰ·������ */
    string u_curDirPath;						/* ��ǰ����Ŀ¼����·�� */

    string u_dirp;				/* ϵͳ���ò���(һ������Pathname)��ָ�� */
    long u_arg[5];				/* ��ŵ�ǰϵͳ���ò��� */
                                /* ϵͳ������س�Ա */
    /*???????????????????????????????*/
    unsigned int u_ar0[5];	    /* ָ�����ջ�ֳ���������EAX�Ĵ���
                                    ��ŵ�ջ��Ԫ�����ֶδ�Ÿ�ջ��Ԫ�ĵ�ַ��
                                    ��V6��r0���ϵͳ���õķ���ֵ���û�����
                                    x86ƽ̨��ʹ��EAX��ŷ���ֵ�����u.ar0[R0] */
    ErrorCode u_error;			/* ��Ŵ����� */

    OpenFiles u_ofiles;		    /* ���̴��ļ������������ */

    IOParameter u_IOParam;	    /* ��¼��ǰ����д�ļ���ƫ�������û�Ŀ�������ʣ���ֽ������� */

    /*?????????????????????????????????????????*/
    FileManager* fileManager;

    string ls;

public:
    User();
    ~User();

    void Ls();
    void Cd(string dirName);
    void Mkdir(string dirName);
    void Create(string fileName, string mode);
    void Delete(string fileName);
    void Open(string fileName, string mode);
    void Close(string fd);
    void Seek(string fd, string offset, string origin);
    void Write(string fd, string inFile, string size);
    void Read(string fd, string outFile, string size);
    //void Pwd();

private:
    bool IsError();
    void EchoError(enum ErrorCode err);
    int INodeMode(string mode);
    int FileMode(string mode);
    bool checkPathName(string path);

};

#endif