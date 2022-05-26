#include "Device.h"

Device::Device()
{
	/* ����ļ������ڣ�����һ���µ��ļ� */
	fp = fopen(deviceFile, "a+");
	fclose(fp);

	/* �Զ�д��ʽ���ļ� */
	fp = fopen(deviceFile, "r+");
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

void Device::OpenD() {
	fp = fopen(deviceFile, "wb+");
	if (fp == NULL) {
		printf("�򿪻��½��ļ�%sʧ�ܣ�", deviceFile);
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
