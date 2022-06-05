#ifndef UTILITY_H
#define UTILITY_H

#define NULL	0

#include<ctime>

class Utility
{
public:
	Utility();
	~Utility();
	/* �Ƚϴ�С */
	static int Min(int a, int b);
	static int Max(int a, int b);

	/* �ַ����Ĺ��ߺ��� */
	static void StringCopy(char* src, char* dst);
	static int StringLength(char* pString);

	/* ���յ��ֽ������� */
	static void ByteCopy(unsigned char* from, unsigned char* to, int count);
	/* �������ֽ������ƣ�С���������ɣ� */
	static void DWordCopy(int* src, int* dst, int count);

	static time_t time(time_t* t);
private:

};

#endif
