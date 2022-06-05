#ifndef BUF_H
#define BUF_H

/*
 * ������ƿ�buf����
 * ��¼����Ӧ�����ʹ���������Ϣ��
 * ͬʱ����I/O����飬��¼�û���
 * ��ص�I/O�����ִ�н����
 */
class Buf
{
public:
	enum BufFlag	/* b_flags�б�־λ */
	{
		B_WRITE = 0x1,		/* д�������������е���Ϣд��Ӳ����ȥ */
		B_READ = 0x2,		/* �����������̶�ȡ��Ϣ�������� */
		B_DONE = 0x4,		/* I/O�������� */
		B_ERROR = 0x8,		/* I/O��������ֹ */
		B_BUSY = 0x10,		/* ��Ӧ��������ʹ���� */
		B_WANTED = 0x20,	/* �н������ڵȴ�ʹ�ø�buf�������Դ����B_BUSY��־ʱ��Ҫ�������ֽ��� */
		B_ASYNC = 0x40,		/* �첽I/O������Ҫ�ȴ������ */
		B_DELWRI = 0x80		/* �ӳ�д������Ӧ����Ҫ��������ʱ���ٽ�������д����Ӧ���豸�� */
	};

	/* Functions */
public:
	Buf();
	~Buf();

	void addFlag(Buf::BufFlag flag);
	void delFlag(Buf::BufFlag flag);
	void clrFlag();

public:
	unsigned int b_flags;	/* ������ƿ��־λ */

	/* ������ƿ���й���ָ�� */
	Buf* av_forw;
	Buf* av_back;

	short	b_dev;			/* �������豸�ţ����и�8λ�����豸�ţ���8λ�Ǵ��豸�� */
	int		b_wcount;		/* �贫�͵��ֽ��� */
	unsigned char* b_addr;	/* ָ��û�����ƿ�������Ļ��������׵�ַ */
	int		b_blkno;		/* �����߼���� */
	int		b_error;		/* I/O����ʱ��Ϣ */
	int		b_resid;		/* I/O����ʱ��δ���͵�ʣ���ֽ��� */

	int no;
};

#endif
