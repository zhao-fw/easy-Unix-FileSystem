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