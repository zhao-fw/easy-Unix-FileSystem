#ifndef UTILITY_H
#define UTILITY_H

#define NULL	0

#include<ctime>

class Utility
{
public:
	Utility();
	~Utility();
	/* 比较大小 */
	static int Min(int a, int b);
	static int Max(int a, int b);

	/* 字符串的工具函数 */
	static void StringCopy(char* src, char* dst);
	static int StringLength(char* pString);

	/* 按照单字节来复制 */
	static void ByteCopy(unsigned char* from, unsigned char* to, int count);
	/* 按照四字节来复制（小端序的问题吧） */
	static void DWordCopy(int* src, int* dst, int count);

	static time_t time(time_t* t);
private:

};

#endif
