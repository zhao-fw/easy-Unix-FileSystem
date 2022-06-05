#define _CRT_SECURE_NO_WARNINGS

#include"program.h"
#include"Utility.h"
#include<fstream>
#include<iostream>

extern FileManager g_FileManager;
extern User g_User;
extern BufferManager g_BufferManager;

void Mkdir(std::string dirName) {
    if (!CheckPathName(dirName)) {
        return;
    }
    g_User.u_arg[1] = Inode::IFDIR;
    g_FileManager.Creat();
    IsError();
}

void Ls() {
    User& u = g_User;
    BufferManager& bufferManager = g_BufferManager;
    /* 用来存储结果 */
    std::string ls_res;

    Inode* pINode = u.u_cdir;
    Buf* pBuf = NULL;
    u.u_IOParam.m_Offset = 0;
    u.u_IOParam.m_Count = pINode->i_size / sizeof(DirectoryEntry);

    /* 当 当前目录中的目录项没有搜索完毕时 */
    while (u.u_IOParam.m_Count) {
        /* 读取物理磁盘 */
        if (0 == u.u_IOParam.m_Offset % Inode::BLOCK_SIZE) {
            if (pBuf) {
                bufferManager.RelseB(pBuf);
            }
            int phyBlkno = pINode->Bmap(u.u_IOParam.m_Offset / Inode::BLOCK_SIZE);
            pBuf = bufferManager.ReadB(phyBlkno);
        }
        memcpy(&u.u_dent, pBuf->b_addr + (u.u_IOParam.m_Offset % Inode::BLOCK_SIZE), sizeof(u.u_dent));
        u.u_IOParam.m_Offset += sizeof(DirectoryEntry);
        u.u_IOParam.m_Count--;

        if (0 == u.u_dent.m_ino)
            continue;
        ls_res += u.u_dent.m_name;
        ls_res += "\n";
    }

    /* 释放缓存 */
    if (pBuf) {
        bufferManager.RelseB(pBuf);
    }
    if (IsError()) {
        return;
    }
    std::cout << ls_res << std::endl;
}

void Cd(std::string dirName) {
    if (!CheckPathName(dirName)) {
        return;
    }
    g_FileManager.ChDir();
    IsError();
}

void Create(std::string fileName, std::string mode) {
    if (!CheckPathName(fileName)) {
        return;
    }
    int mod = InodeMode(mode);
    if (mod == 0) {
        std::cout << "文件权限设置不正确! " << std::endl;
        return;
    }

    g_User.u_arg[1] = mod;
    g_FileManager.Creat();
    IsError();
}

void Delete(std::string fileName) {
    if (!CheckPathName(fileName)) {
        return;
    }
    g_FileManager.UnLink();
    IsError();
}

void Open(std::string fileName, std::string mode) {
    if (!CheckPathName(fileName)) {
        return;
    }
    int mod = FileMode(mode);
    if (mod == 0) {
        std::cout << "文件权限设置不正确!" << std::endl;
        return;
    }

    g_User.u_arg[1] = mod;
    g_FileManager.Open();
    if (IsError())
        return;
    std::cout << "文件打开成功，文件fd为：" << g_User.u_ar0[User::EAX] << std::endl;
}

void Close(std::string sfd) {
    if (sfd.empty() || !isdigit(sfd.front())) {
        std::cout << "参数fd不正确!" << std::endl;
        return;
    }
    g_User.u_arg[0] = stoi(sfd);;
    g_FileManager.Close();
    IsError();
}

void Seek(std::string sfd, std::string offset, std::string origin) {
    if (sfd.empty() || !isdigit(sfd.front())) {
        std::cout << "参数fd不正确!" << std::endl;
        return;
    }
    if (offset.empty()) {
        std::cout << "缺少offset参数!" << std::endl;
        return;
    }
    if (origin.empty() || !isdigit(origin.front())) {
        std::cout << "参数origin不正确!" << std::endl;
        return;
    }
    g_User.u_arg[0] = stoi(sfd);
    g_User.u_arg[1] = stoi(offset);
    g_User.u_arg[2] = stoi(origin);
    g_FileManager.Seek();
    IsError();
}

void Write(std::string sfd, std::string inFile, std::string size) {
    if (sfd.empty() || !isdigit(sfd.front())) {
        std::cout << "参数fd不正确!" << std::endl;
        return;
    }
    int fd = stoi(sfd);

    int usize;

    if (size.empty()) {
        /* 将某个文件全部内容写入 */
        std::fstream fin(inFile, std::ios::in | std::ios::binary);
        fin.seekg(0, std::ios::end);
        int length = fin.tellg();
        fin.seekg(0, std::ios::beg);
        char* buffer = new char[length];

        if (!fin) {
            std::cout << "打开文件 " << inFile << " 失败!" << std::endl;
            return;
        }
        fin.read(buffer, length);
        fin.close();

        g_User.u_arg[0] = fd;
        g_User.u_arg[1] = (long)buffer;
        g_User.u_arg[2] = length;
        g_FileManager.Write();

        if (IsError())
            return;
        std::cout << "成功写入 " << g_User.u_ar0[User::EAX] << "字节!" << std::endl;
        delete[]buffer;
    }
    else if ((usize = stoi(size)) < 0) {
        std::cout << "参数size不正确!" << std::endl;
        return;
    }
    else {
        /* 按大小写入 */
        char* buffer = new char[usize];
        std::fstream fin(inFile, std::ios::in | std::ios::binary);
        if (!fin) {
            std::cout << "打开文件 " << inFile << " 失败!" << std::endl;
            return;
        }
        fin.read(buffer, usize);
        fin.close();

        g_User.u_arg[0] = fd;
        g_User.u_arg[1] = (long)buffer;
        g_User.u_arg[2] = usize;
        g_FileManager.Write();

        if (IsError())
            return;
        std::cout << "成功写入 " << g_User.u_ar0[User::EAX] << "字节!" << std::endl;
        delete[]buffer;
    }
    return;
}

void Read(std::string sfd, std::string outFile, std::string size) {
    if (sfd.empty() || !isdigit(sfd.front())) {
        std::cout << "参数fd不正确!" << std::endl;
        return;
    }
    int fd = stoi(sfd);

    int usize;
    if (size.empty() || !isdigit(size.front()) || (usize = stoi(size)) < 0) {
        std::cout << "参数size不正确!" << std::endl;
        return;
    }
    char* buffer = new char[usize];
    g_User.u_arg[0] = fd;
    g_User.u_arg[1] = (long)buffer;
    g_User.u_arg[2] = usize;
    g_FileManager.Read();
    if (IsError())
        return;

    std::cout << "成功读取 " << g_User.u_ar0[User::EAX] << "字节：" << std::endl;
    if (outFile.empty()) {
        for (unsigned int i = 0; i < g_User.u_ar0[User::EAX] && i < usize; ++i) {
            std::cout << (char)buffer[i];
        }
        std::cout << std::endl;
        return;
    }
    std::fstream fout(outFile, std::ios::out | std::ios::binary);
    if (!fout) {
        std::cout << "打开文件 " << outFile << " 失败!" << std::endl;
        return;
    }
    fout.write(buffer, g_User.u_ar0[User::EAX]);
    fout.close();
    std::cout << "读取结果成功保存到文件 " << outFile << " 中!" << std::endl;
    delete[]buffer;
}

int InodeMode(std::string mode) {
    int mod = 0;
    if (mode.find("-r") != std::string::npos) {
        mod |= Inode::IREAD;
    }
    if (mode.find("-w") != std::string::npos) {
        mod |= Inode::IWRITE;
    }
    if (mode.find("-rw") != std::string::npos) {
        mod |= (Inode::IREAD | Inode::IWRITE);
    }
    return mod;
}

int FileMode(std::string mode) {
    int mod = 0;
    if (mode.find("-r") != std::string::npos) {
        mod |= File::FREAD;
    }
    if (mode.find("-w") != std::string::npos) {
        mod |= File::FWRITE;
    }
    if (mode.find("-rw") != std::string::npos) {
        mod |= (File::FREAD | File::FWRITE);
    }
    return mod;
}

bool CheckPathName(std::string path) {
    std::string tmp;
    char* path_c = new char[path.size() + 1];
    strcpy(path_c, path.c_str());
    g_User.u_arg[0] = (int)path_c;

    if (path.empty()) {
        std::cout << "参数path设置不正确!" << std::endl;
        return false;
    }
    
    if (path.substr(0, 2) != "..") {
        tmp = path;
    }
    else {
        std::string pre = g_User.u_curdir;
        unsigned int p = 0;
        /* 可以多重相对路径 */
        for (; pre.length() > 3 && p < path.length() && path.substr(p, 2) == ".."; ) {
            pre.pop_back();
            pre.erase(pre.find_last_of('/') + 1);
            p += 2;
            p += p < path.length() && path[p] == '/';
        }
        tmp = pre + path.substr(p);
    }

    /* 忽略最后的字符 '/' */
    if (tmp.length() > 1 && tmp.back() == '/') {
        tmp.pop_back();
    }

    g_User.u_dirp = (char*)g_User.u_arg[0];

    int n = tmp.find_last_of('/');
    if (tmp.length() - n > DirectoryEntry::DIRSIZ) {
        std::cout << "the fileName or dirPath can't be greater than 28 size ! \n";
        return false;
    }
    else {
        return true;
    }
}

bool IsError() {
    if (g_User.u_error != User::U_NOERROR) {
        std::cout << "errno = " << g_User.u_error;
        EchoError(g_User.u_error);
        g_User.u_error = User::U_NOERROR;
        return true;
    }
    return false;
}

void EchoError(User::ErrorCode err) {
    std::string estr;
    switch (err) {
    case User::U_NOERROR:
        estr = " No u_error ";
        break;
    case User::U_ENOENT:
        estr = " No such file or directory ";
        break;
    case User::U_EBADF:
        estr = " Bad file number ";
        break;
    case User::U_EACCES:
        estr = " Permission denied ";
        break;
    case User::U_ENOTDIR:
        estr = " Not a directory ";
        break;
    case User::U_ENFILE:
        estr = " File table overflow ";
        break;
    case User::U_EMFILE:
        estr = " Too many open files ";
        break;
    case User::U_EFBIG:
        estr = " File too large ";
        break;
    case User::U_ENOSPC:
        estr = " No space left on device ";
        break;
    default:
        break;
    }
    std::cout << estr << std::endl;
}