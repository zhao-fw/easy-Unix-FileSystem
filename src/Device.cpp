#define _CRT_SECURE_NO_WARNINGS

#include "Device.h"

Device::Device()
{
	/* 如果文件不存在，创建一个新的文件 */
	fp = fopen(deviceFile, "a+");
	fclose(fp);

	/* 以读写方式打开文件 */
	fp = fopen(deviceFile, "rb+");
}

Device::~Device()
{
	if (fp) {
		fclose(fp);
	}
}

bool Device::Exist() {
	return fp != NULL;
}

void Device::NewD() {
	fp = fopen(deviceFile, "wb+");
	if (fp == NULL) {
		printf("打开或新建文件%s失败！", deviceFile);
		exit(-1);
	}
}

void Device::WriteD(const void* buffer, unsigned int size, int offset) {
	if (offset >= 0) {
		fseek(fp, offset, SEEK_SET);
	}
	fwrite(buffer, size, 1, fp);
}

void Device::ReadD(void* buffer, unsigned int size, int offset) {
	if (offset >= 0) {
		fseek(fp, offset, SEEK_SET);
	}
	fread(buffer, size, 1, fp);
}
