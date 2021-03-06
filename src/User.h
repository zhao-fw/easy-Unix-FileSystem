#ifndef USER_H
#define USER_H

#include "FileManager.h"
#include <string>

class User {
public:
    static const int EAX = 0;	/* u.ar0[EAX]；访问现场保护区中EAX寄存器的偏移量 */

    enum ErrorCode {
        U_NOERROR = 0,  	/* No u_error */
        U_EPERM = 1,	    /* Operation not permitted */
        U_ENOENT = 2,	    /* No such file or directory */
        U_ESRCH = 3,	    /* No such process */
        U_EINTR = 4,	    /* Interrupted system call */
        U_EIO = 5,	    /* I/O u_error */
        U_ENXIO = 6,	    /* No such device or address */
        U_E2BIG = 7,	    /* Arg list too long */
        U_ENOEXEC = 8,	/* Exec format u_error */
        U_EBADF = 9,	    /* Bad file number */
        U_ECHILD = 10,	/* No child processes */
        U_EAGAIN = 11,	/* Try again */
        U_ENOMEM = 12,	/* Out of memory */
        U_EACCES = 13,	    /* Permission denied */
        U_EFAULT = 14,	/* Bad address */
        U_ENOTBLK = 15,	/* Block device required */
        U_EBUSY = 16,	    /* Device or resource busy */
        U_EEXIST = 17,	/* File exists */
        U_EXDEV = 18,	    /* Cross-device link */
        U_ENODEV = 19,	/* No such device */
        U_ENOTDIR = 20,	    /* Not a directory */
        U_EISDIR = 21,	/* Is a directory */
        U_EINVAL = 22,	/* Invalid argument */
        U_ENFILE = 23,	    /* File table overflow */
        U_EMFILE = 24,	    /* Too many open files */
        U_ENOTTY = 25,	/* Not a typewriter(terminal) */
        U_ETXTBSY = 26,	/* Text file busy */
        U_EFBIG = 27,	    /* File too large */
        U_ENOSPC = 28,	    /* No space left on device */
        U_ESPIPE = 29,	/* Illegal seek */
        U_EROFS = 30,	    /* Read-only file system */
        U_EMLINK = 31,	/* Too many links */
        U_EPIPE = 32,	    /* Broken pipe */
        U_ENOSYS = 100
    };

public:
    /* 系统调用相关成员 */
    unsigned int u_ar0[5];	/* 指向核心栈现场保护区中EAX寄存器
                            存放的栈单元，本字段存放该栈单元的地址。
                            在V6中r0存放系统调用的返回值给用户程序，
                            x86平台上使用EAX存放返回值，替代u.u_ar0[R0] */

    int u_arg[5];			/* 存放当前系统调用参数 */
    char* u_dirp;			/* 系统调用参数(一般用于Pathname)的指针 */

    Inode* u_cdir;		/* 指向当前目录的Inode指针 */
    Inode* u_pdir;		/* 指向父目录的Inode指针 */

    DirectoryEntry u_dent;					/* 当前目录的目录项 */
    char u_dbuf[DirectoryEntry::DIRSIZ];	/* 当前路径分量 */
    char u_curdir[128];				            /* 当前工作目录完整路径 */

    User::ErrorCode u_error;			/* 存放错误码 */

    OpenFiles u_ofiles;		    /* 进程打开文件描述符表对象 */

    IOParameter u_IOParam;	    /* 记录当前读、写文件的偏移量，用户目标区域和剩余字节数参数 */
public:
    User();
    ~User();
};

#endif