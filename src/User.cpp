#define _CRT_SECURE_NO_WARNINGS
#include"User.h"

extern FileManager g_FileManager;

User::User()
{
    u_error = U_NOERROR;
    strcpy(u_curdir, "/");
    u_dirp = u_curdir;
    u_cdir = g_FileManager.rootDirInode;
    u_pdir = NULL;
    memset(u_arg, 0, sizeof(u_arg));
}

User::~User()
{
}

