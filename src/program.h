#ifndef PROGRAM_H
#define PROGRAM_H

#include"FileManager.h"
#include"User.h"

/* 输出错误信息 */
bool IsError();
void EchoError(enum User::ErrorCode err);

/* 将Inode的权限由字符串转化为int类型 */
int InodeMode(std::string mode);

/* 将文件权限由字符串转化为int类型 */
int FileMode(std::string mode);

/* 处理传入的路径信息，并将结果传递给User */
bool CheckPathName(std::string path);

/* 列出当前目录的文件 */
void Ls();

/* 切换位置 */
void Cd(std::string dirName);

/* 创建文件夹 */
void Mkdir(std::string dirName);

/* 创建文件 */
void Create(std::string fileName, std::string mode);

/* 删除文件 */
void Delete(std::string fileName);

/* 打开文件 */
void Open(std::string fileName, std::string mode);

/* 关闭文件 */
void Close(std::string fd);

/* 移动文件指针 */
void Seek(std::string fd, std::string offset, std::string origin);

/* 写入文件 */
void Write(std::string fd, std::string inFile, std::string size);

/* 读取文件 */
void Read(std::string fd, std::string outFile, std::string size);


#endif
