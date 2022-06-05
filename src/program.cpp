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
    /* �����洢��� */
    std::string ls_res;

    Inode* pINode = u.u_cdir;
    Buf* pBuf = NULL;
    u.u_IOParam.m_Offset = 0;
    u.u_IOParam.m_Count = pINode->i_size / sizeof(DirectoryEntry);

    /* �� ��ǰĿ¼�е�Ŀ¼��û���������ʱ */
    while (u.u_IOParam.m_Count) {
        /* ��ȡ������� */
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

    /* �ͷŻ��� */
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
        std::cout << "�ļ�Ȩ�����ò���ȷ! " << std::endl;
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
        std::cout << "�ļ�Ȩ�����ò���ȷ!" << std::endl;
        return;
    }

    g_User.u_arg[1] = mod;
    g_FileManager.Open();
    if (IsError())
        return;
    std::cout << "�ļ��򿪳ɹ����ļ�fdΪ��" << g_User.u_ar0[User::EAX] << std::endl;
}

void Close(std::string sfd) {
    if (sfd.empty() || !isdigit(sfd.front())) {
        std::cout << "����fd����ȷ!" << std::endl;
        return;
    }
    g_User.u_arg[0] = stoi(sfd);;
    g_FileManager.Close();
    IsError();
}

void Seek(std::string sfd, std::string offset, std::string origin) {
    if (sfd.empty() || !isdigit(sfd.front())) {
        std::cout << "����fd����ȷ!" << std::endl;
        return;
    }
    if (offset.empty()) {
        std::cout << "ȱ��offset����!" << std::endl;
        return;
    }
    if (origin.empty() || !isdigit(origin.front())) {
        std::cout << "����origin����ȷ!" << std::endl;
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
        std::cout << "����fd����ȷ!" << std::endl;
        return;
    }
    int fd = stoi(sfd);

    int usize;

    if (size.empty()) {
        /* ��ĳ���ļ�ȫ������д�� */
        std::fstream fin(inFile, std::ios::in | std::ios::binary);
        fin.seekg(0, std::ios::end);
        int length = fin.tellg();
        fin.seekg(0, std::ios::beg);
        char* buffer = new char[length];

        if (!fin) {
            std::cout << "���ļ� " << inFile << " ʧ��!" << std::endl;
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
        std::cout << "�ɹ�д�� " << g_User.u_ar0[User::EAX] << "�ֽ�!" << std::endl;
        delete[]buffer;
    }
    else if ((usize = stoi(size)) < 0) {
        std::cout << "����size����ȷ!" << std::endl;
        return;
    }
    else {
        /* ����Сд�� */
        char* buffer = new char[usize];
        std::fstream fin(inFile, std::ios::in | std::ios::binary);
        if (!fin) {
            std::cout << "���ļ� " << inFile << " ʧ��!" << std::endl;
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
        std::cout << "�ɹ�д�� " << g_User.u_ar0[User::EAX] << "�ֽ�!" << std::endl;
        delete[]buffer;
    }
    return;
}

void Read(std::string sfd, std::string outFile, std::string size) {
    if (sfd.empty() || !isdigit(sfd.front())) {
        std::cout << "����fd����ȷ!" << std::endl;
        return;
    }
    int fd = stoi(sfd);

    int usize;
    if (size.empty() || !isdigit(size.front()) || (usize = stoi(size)) < 0) {
        std::cout << "����size����ȷ!" << std::endl;
        return;
    }
    char* buffer = new char[usize];
    g_User.u_arg[0] = fd;
    g_User.u_arg[1] = (long)buffer;
    g_User.u_arg[2] = usize;
    g_FileManager.Read();
    if (IsError())
        return;

    std::cout << "�ɹ���ȡ " << g_User.u_ar0[User::EAX] << "�ֽڣ�" << std::endl;
    if (outFile.empty()) {
        for (unsigned int i = 0; i < g_User.u_ar0[User::EAX] && i < usize; ++i) {
            std::cout << (char)buffer[i];
        }
        std::cout << std::endl;
        return;
    }
    std::fstream fout(outFile, std::ios::out | std::ios::binary);
    if (!fout) {
        std::cout << "���ļ� " << outFile << " ʧ��!" << std::endl;
        return;
    }
    fout.write(buffer, g_User.u_ar0[User::EAX]);
    fout.close();
    std::cout << "��ȡ����ɹ����浽�ļ� " << outFile << " ��!" << std::endl;
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
        std::cout << "����path���ò���ȷ!" << std::endl;
        return false;
    }
    
    if (path.substr(0, 2) != "..") {
        tmp = path;
    }
    else {
        std::string pre = g_User.u_curdir;
        unsigned int p = 0;
        /* ���Զ������·�� */
        for (; pre.length() > 3 && p < path.length() && path.substr(p, 2) == ".."; ) {
            pre.pop_back();
            pre.erase(pre.find_last_of('/') + 1);
            p += 2;
            p += p < path.length() && path[p] == '/';
        }
        tmp = pre + path.substr(p);
    }

    /* ���������ַ� '/' */
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