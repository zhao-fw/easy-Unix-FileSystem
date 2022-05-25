#ifndef UTILITY_H
#define UTILITY_H

#define NULL	0

class Utility
{
public:
	Utility();
	~Utility();
	/* 比较大小 */
	static int Min(int a, int b);
	static int Max(int a, int b);

	/* 按照单字节来复制 */
	static void ByteCopy(unsigned char* from, unsigned char* to, int count);
	/* 按照四字节来复制（小端序的问题吧） */
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
