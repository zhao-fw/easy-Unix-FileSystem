#include"Utility.h"

int Utility::Min(int a, int b) {
	return (a < b) ? a : b;
}

int Utility::Max(int a, int b) {
	return (a > b) ? a : b;
}

void Utility::ByteCopy(unsigned char* from, unsigned char* to, int count) {
	while (count--)
	{
		*to++ = *from++;
	}
	return;
}

void Utility::DWordCopy(int* src, int* dst, int count)
{
	while (count--)
	{
		*dst++ = *src++;
	}
	return;
}

void Utility::StringCopy(char* src, char* dst)
{
	while ((*dst++ = *src++) != 0);
}

int Utility::StringLength(char* pString)
{
	int length = 0;
	char* pChar = pString;

	while (*pChar++)
	{
		length++;
	}

	/* ·µ»Ø×Ö·û´®³¤¶È */
	return length;
}

time_t Utility::time(time_t* t) {
	return ::time(t);
}
