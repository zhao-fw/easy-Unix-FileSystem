#ifndef UTILITY_H
#define UTILITY_H

#define NULL	0

class Utility
{
public:
	Utility();
	~Utility();
	/* �Ƚϴ�С */
	static int Min(int a, int b);
	static int Max(int a, int b);

	/* ���յ��ֽ������� */
	static void ByteCopy(unsigned char* from, unsigned char* to, int count);
	/* �������ֽ������ƣ�С���������ɣ� */
	static void DWordCopy(int* src, int* dst, int count);
private:

};

Utility::Utility()
{
}

Utility::~Utility()
{
}

#endif
