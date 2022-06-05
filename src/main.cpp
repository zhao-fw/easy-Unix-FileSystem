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
        "Command       :  man - 显示帮助手册 \n"
        "Usage         :  man [命令] \n"
        "Parameter     :  [命令] 如下：  \n"
        "                 man          :  手册 \n"
        "                 fformat      :  格式化 \n"
        "                 exit         :  退出 \n"
        "                 mkdir        :  新建目录 \n"
        "                 cd           :  改变目录 \n"
        "                 ls           :  列出目录及文件 \n"
        "                 create       :  新建文件 \n"
        "                 delete       :  删除文件 \n"
        "                 open         :  打开文件 \n"
        "                 close        :  关闭文件 \n"
        "                 seek         :  移动读写指针 \n"
        "                 write        :  写入文件 \n"
        "                 read         :  读取文件 \n"
        ;

    static std::string fformat =
        "Command       :  fformat - 进行文件系统格式化 \n"
        "Description   :  将整个文件系统进行格式化，即清空所有文件及目录! \n"
        "Usage         :  fformat \n"
        "Parameter     :  无 \n"
        ;

    static std::string exit =
        "Command       :  exit - 退出文件系统 \n"
        "Description   :  该指令实现程序的正确退出，保证磁盘文件得到更新。\n"
        "Usage         :  exit \n"
        "Parameter     :  无 \n"
        ;

    static std::string mkdir =
        "Command       :  mkdir - 建立目录 \n"
        "Description   :  新建一个目录。 \n"
        "Usage         :  mkdir <目录名> \n"
        "Parameter     :  <目录名> 可以是相对路径，也可以是绝对路径 \n"
        ;

    static std::string ls =
        "Command       :  ls - 列出当前目录内容 \n"
        "Description   :  列出当前目录中包含的文件名或目录名。 \n"
        "Usage         :  ls \n"
        "Parameter     :  无 \n"
        ;

    static std::string cd =
        "Command       :  cd - 改变当前目录 \n"
        "Description   :  改变当前工作目录。\n"
        "Usage         :  cd <目录名> \n"
        "Parameter     :  <目录名>可以从当前目录出发，也可以从根目录出发；\n"
        ;

    static std::string create =
        "Command       :  create - 新建文件 \n"
        "Description   :  新建一个文件。 \n"
        "Usage         :  create <文件名> <选项> \n"
        "Parameter     :  <文件名> \n"
        "                 <选项> -r 只读属性 \n"
        "                 <选项> -w 只写属性 \n"
        "                 <选项> -rw == -r -w 读写属性 \n"
        ;

    static std::string delet =
        "Command       :  delete - 删除文件 \n"
        "Description   :  删除一个文件。 \n"
        "Usage         :  delete <文件名> \n"
        "Parameter     :  <文件名> \n"
        ;

    static std::string open =
        "Command       :  open - 打开文件 \n"
        "Description   :  打开一个文件。若要进行文件的读写必须先open。\n"
        "Usage         :  open <文件名> <选项> \n"
        "Parameter     :  <文件名> \n"
        "                 <选项> -r 只读属性 \n"
        "                 <选项> -w 只写属性 \n"
        "                 <选项> -rw == -r -w 读写属性 \n"
        ;

    static std::string close =
        "Command       :  close - 关闭文件 \n"
        "Description   :  关闭一个文件。\n"
        "Usage         :  close <file descriptor> \n"
        "Parameter     :  <file descriptor> 文件描述符 \n"
        ;

    static std::string seek =
        "Command       :  seek - 写入文件 \n"
        "Description   :  移动某个打开文件的读写指针位置 \n"
        "Usage         :  seek <file descriptor> <offset> <origin> \n"
        "Parameter     :  <file descriptor> open返回的文件描述符 \n"
        "                 <offset> 指定从 <origin> 开始的偏移量 可正可负 \n"
        "                 <origin> 指定起始位置 可为0(SEEK_SET), 1(SEEK_CUR), 2(SEEK_END) \n"
        ;

    static std::string write =
        "Command       :  write - 写入文件 \n"
        "Description   :  写入一个已经打开的文件中。 \n"
        "Usage         :  write <file descriptor> <InFileName> [<size>] \n"
        "Parameter     :  <file descriptor> open返回的文件描述符 \n"
        "                 <InFileName> 指定写入内容为文件InFileName中的内容 \n"
        "                 <size> 指定写入字节数，大小为 <size>, 如果不指定，则将文件的全部内容写入 \n"
        ;

    static std::string read =
        "Command       :  read -读取文件 \n"
        "Description   :  从一个已经打开的文件中读取。 \n"
        "Usage         :  read <file descriptor> [-o <OutFileName>] <size> \n"
        "Parameter     :  <file descriptor> open返回的文件描述符 \n"
        "                 [-o <OutFileName>] -o 指定输出方式为文件，文件名为 <OutFileName> ,不指定为shell \n"
        "                 <size> 指定读取字节数，大小为 <size> \n"
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
        std::cout << "shell : " << command << " : 指令不存在" << std::endl;
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
    std::cout << "-------------------- 类Unix简易文件系统 --------------------" << std::endl;

    while (1) {
        /* 指令不为空 */
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
                std::cout << "shell : " << cmd << " : 指令不存在" << std::endl;
            }
        }

        /* 等待下次输入 */
        std::cout << "[zyf@unix-fs " << user->u_curdir << " ]$ ";
        getline(std::cin, line);
    }
    return 0;
}