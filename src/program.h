#ifndef PROGRAM_H
#define PROGRAM_H

#include"FileManager.h"
#include"User.h"

/* ���������Ϣ */
bool IsError();
void EchoError(enum User::ErrorCode err);

/* ��Inode��Ȩ�����ַ���ת��Ϊint���� */
int InodeMode(std::string mode);

/* ���ļ�Ȩ�����ַ���ת��Ϊint���� */
int FileMode(std::string mode);

/* �������·����Ϣ������������ݸ�User */
bool CheckPathName(std::string path);

/* �г���ǰĿ¼���ļ� */
void Ls();

/* �л�λ�� */
void Cd(std::string dirName);

/* �����ļ��� */
void Mkdir(std::string dirName);

/* �����ļ� */
void Create(std::string fileName, std::string mode);

/* ɾ���ļ� */
void Delete(std::string fileName);

/* ���ļ� */
void Open(std::string fileName, std::string mode);

/* �ر��ļ� */
void Close(std::string fd);

/* �ƶ��ļ�ָ�� */
void Seek(std::string fd, std::string offset, std::string origin);

/* д���ļ� */
void Write(std::string fd, std::string inFile, std::string size);

/* ��ȡ�ļ� */
void Read(std::string fd, std::string outFile, std::string size);


#endif
