#include "INode.h"
#include "File.h"
#include "FileSystem.h"
#include "OpenFileManager.h"
#include "FileManager.h"
#include "User.h"
#include "program.h"
#include <iostream>
#include <unordered_map>

Device g_Device;
BufferManager g_BufferManager;
OpenFileTable g_OpenFileTable;
SuperBlock g_SuperBlock;
FileSystem g_FileSystem;
InodeTable g_InodeTable;
FileManager g_FileManager;
User g_User;

void man(std::string command) {

    static std::string man =
        "Command       :  man - ��ʾ�����ֲ� \n"
        "Usage         :  man [����] \n"
        "Parameter     :  [����] ���£�  \n"
        "                 man          :  �ֲ� \n"
        "                 fformat      :  ��ʽ�� \n"
        "                 exit         :  �˳� \n"
        "                 mkdir        :  �½�Ŀ¼ \n"
        "                 cd           :  �ı�Ŀ¼ \n"
        "                 ls           :  �г�Ŀ¼���ļ� \n"
        "                 create       :  �½��ļ� \n"
        "                 delete       :  ɾ���ļ� \n"
        "                 open         :  ���ļ� \n"
        "                 close        :  �ر��ļ� \n"
        "                 seek         :  �ƶ���дָ�� \n"
        "                 write        :  д���ļ� \n"
        "                 read         :  ��ȡ�ļ� \n"
        ;

    static std::string fformat =
        "Command       :  fformat - �����ļ�ϵͳ��ʽ�� \n"
        "Description   :  �������ļ�ϵͳ���и�ʽ��������������ļ���Ŀ¼! \n"
        "Usage         :  fformat \n"
        "Parameter     :  �� \n"
        ;

    static std::string exit =
        "Command       :  exit - �˳��ļ�ϵͳ \n"
        "Description   :  ��ָ��ʵ�ֳ������ȷ�˳�����֤�����ļ��õ����¡�\n"
        "Usage         :  exit \n"
        "Parameter     :  �� \n"
        ;

    static std::string mkdir =
        "Command       :  mkdir - ����Ŀ¼ \n"
        "Description   :  �½�һ��Ŀ¼�� \n"
        "Usage         :  mkdir <Ŀ¼��> \n"
        "Parameter     :  <Ŀ¼��> ���������·����Ҳ�����Ǿ���·�� \n"
        ;

    static std::string ls =
        "Command       :  ls - �г���ǰĿ¼���� \n"
        "Description   :  �г���ǰĿ¼�а������ļ�����Ŀ¼���� \n"
        "Usage         :  ls \n"
        "Parameter     :  �� \n"
        ;

    static std::string cd =
        "Command       :  cd - �ı䵱ǰĿ¼ \n"
        "Description   :  �ı䵱ǰ����Ŀ¼��\n"
        "Usage         :  cd <Ŀ¼��> \n"
        "Parameter     :  <Ŀ¼��>���Դӵ�ǰĿ¼������Ҳ���ԴӸ�Ŀ¼������\n"
        ;

    static std::string create =
        "Command       :  create - �½��ļ� \n"
        "Description   :  �½�һ���ļ��� \n"
        "Usage         :  create <�ļ���> <ѡ��> \n"
        "Parameter     :  <�ļ���> \n"
        "                 <ѡ��> -r ֻ������ \n"
        "                 <ѡ��> -w ֻд���� \n"
        "                 <ѡ��> -rw == -r -w ��д���� \n"
        ;

    static std::string delet =
        "Command       :  delete - ɾ���ļ� \n"
        "Description   :  ɾ��һ���ļ��� \n"
        "Usage         :  delete <�ļ���> \n"
        "Parameter     :  <�ļ���> \n"
        ;

    static std::string open =
        "Command       :  open - ���ļ� \n"
        "Description   :  ��һ���ļ�����Ҫ�����ļ��Ķ�д������open��\n"
        "Usage         :  open <�ļ���> <ѡ��> \n"
        "Parameter     :  <�ļ���> \n"
        "                 <ѡ��> -r ֻ������ \n"
        "                 <ѡ��> -w ֻд���� \n"
        "                 <ѡ��> -rw == -r -w ��д���� \n"
        ;

    static std::string close =
        "Command       :  close - �ر��ļ� \n"
        "Description   :  �ر�һ���ļ���\n"
        "Usage         :  close <file descriptor> \n"
        "Parameter     :  <file descriptor> �ļ������� \n"
        ;

    static std::string seek =
        "Command       :  seek - д���ļ� \n"
        "Description   :  �ƶ�ĳ�����ļ��Ķ�дָ��λ�� \n"
        "Usage         :  seek <file descriptor> <offset> <origin> \n"
        "Parameter     :  <file descriptor> open���ص��ļ������� \n"
        "                 <offset> ָ���� <origin> ��ʼ��ƫ���� �����ɸ� \n"
        "                 <origin> ָ����ʼλ�� ��Ϊ0(SEEK_SET), 1(SEEK_CUR), 2(SEEK_END) \n"
        ;

    static std::string write =
        "Command       :  write - д���ļ� \n"
        "Description   :  д��һ���Ѿ��򿪵��ļ��С� \n"
        "Usage         :  write <file descriptor> <InFileName> [<size>] \n"
        "Parameter     :  <file descriptor> open���ص��ļ������� \n"
        "                 <InFileName> ָ��д������Ϊ�ļ�InFileName�е����� \n"
        "                 <size> ָ��д���ֽ�������СΪ <size>, �����ָ�������ļ���ȫ������д�� \n"
        ;

    static std::string read =
        "Command       :  read -��ȡ�ļ� \n"
        "Description   :  ��һ���Ѿ��򿪵��ļ��ж�ȡ�� \n"
        "Usage         :  read <file descriptor> [-o <OutFileName>] <size> \n"
        "Parameter     :  <file descriptor> open���ص��ļ������� \n"
        "                 [-o <OutFileName>] -o ָ�������ʽΪ�ļ����ļ���Ϊ <OutFileName> ,��ָ��Ϊshell \n"
        "                 <size> ָ����ȡ�ֽ�������СΪ <size> \n"
        ;

    static std::unordered_map<std::string, std::string*>manMap({
        { "man", &man },
        { "fformat", &fformat },
        { "exit", &exit },
        { "mkdir", &mkdir },
        { "cd", &cd },
        { "ls", &ls },
        { "create", &create },
        { "delete", &delet },
        { "open", &open },
        { "close", &close },
        { "seek", &seek },
        { "write", &write },
        { "read", &read },
        });

    auto it = manMap.find(command);
    if (it == manMap.end()) {
        std::cout << "shell : " << command << " : ָ�����" << std::endl;
        return;
    }
    std::cout << *it->second;

    /*
    for (auto it = manMap.begin(); it != manMap.end(); it++) {
        std::cout << "[" << it->first << "]" << std::endl;
        std::cout << *it->second;
        std::cout << std::endl;
    }
    */
}

void cmdArgs(const std::string& cmd, std::vector<std::string>& args) {
    args.clear();
    std::string str;
    unsigned int p, q;
    for (p = 0, q = 0; q < cmd.length(); p = q + 1) {
        q = cmd.find_first_of(" \n", p);
        str = cmd.substr(p, q - p);
        if (!str.empty()) {
            args.push_back(str);
        }
        if (q == std::string::npos)
            return;
    }
}

int main() {
    User* user = &g_User;
    std::string line = "man";
    std::vector<std::string> args;
    std::string cmd, arg1, arg2, arg3;
    std::cout << "-------------------- ��Unix�����ļ�ϵͳ --------------------" << std::endl;

    while (1) {
        /* ָ�Ϊ�� */
        if (line != "") {
            cmdArgs(line, args);
            cmd = args[0];
            arg1 = args.size() > 1 ? args[1] : "";
            arg2 = args.size() > 2 ? args[2] : "";
            arg3 = args.size() > 3 ? args[3] : "";
            if (cmd == "man") {
                man(arg1.empty() ? "man" : arg1);
            }
            else if (cmd == "fformat") {
                g_OpenFileTable.Format();
                g_InodeTable.Format();
                g_BufferManager.FormatBuf();
                g_FileSystem.FormatDevice();
                exit(0);
            }
            else if (cmd == "exit") {
                exit(0);
            }
            else if (cmd == "mkdir") {
                Mkdir(args[1]);
            }
            else if (cmd == "ls") {
                Ls();
            }
            else if (cmd == "cd") {
                Cd(arg1);
            }
            else if (cmd == "create") {
                Create(arg1, arg2 + arg3);
            }
            else if (cmd == "delete") {
                Delete(arg1);
            }
            else if (cmd == "open") {
                Open(arg1, line);
            }
            else if (cmd == "close") {
                Close(arg1);
            }
            else if (cmd == "seek") {
                Seek(arg1, arg2, arg3);
            }
            else if (cmd == "read") {
                if (arg2 == "-o")
                    Read(arg1, arg3, args[4]);
                else
                    Read(arg1, "", arg2);
            }
            else if (cmd == "write") {
                Write(arg1, arg2, arg3);
            }
            else if (cmd != "") {
                std::cout << "shell : " << cmd << " : ָ�����" << std::endl;
            }
        }

        /* �ȴ��´����� */
        std::cout << "[zyf@unix-fs " << user->u_curdir << " ]$ ";
        getline(std::cin, line);
    }
    return 0;
}