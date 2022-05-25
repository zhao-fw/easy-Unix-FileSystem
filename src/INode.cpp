#include "Utility.h"
#include "INode.h"
#include "FileSystem.h"
#include "User.h"

/*
 * ���ڲ���Ҫʵ��kernel�����ݣ����Զ�����kernel�ж���� g_BufferManager, g_FileSystem, g_user�����ݣ�
 * ֱ����main�ж����ȫ�ֱ�����ʵ�֡�
 */
extern BufferManager g_BufferManager;
extern FileSystem g_FileSystem;
extern User g_User;

/*==============================class Inode===================================*/

Inode::Inode()
{
	 /* ��Inode����ĳ�Ա������ʼ��Ϊ��Чֵ */
	this->i_number = -1;
	this->i_flag = 0;
	this->i_count = 0;

	this->i_mode = 0;
	this->i_nlink = 0;
	this->i_uid = -1;
	this->i_gid = -1;
	this->i_size = 0;
	this->i_lastr = -1;
	for (int i = 0; i < 10; i++)
		this->i_addr[i] = 0;
}

Inode::~Inode()
{
	//nothing to do here
}

void Inode::ReadI()
{
	int lbn;	/* �ļ��߼���� */
	int bn;		/* lbn��Ӧ�������̿�� */
	int offset;	/* ��ǰ�ַ�������ʼ����λ�� */
	int nbytes;	/* �������û�Ŀ�����ֽ����� */
	Buf* pBuf;
	User& u = g_User;
	BufferManager& bufMgr = g_BufferManager;

	if (0 == u.u_IOParam.m_Count)
	{
		/* ��Ҫ���ֽ���Ϊ�㣬�򷵻� */
		return;
	}

	this->i_flag |= Inode::IACC;

	/* һ��һ���ַ���ض�������ȫ�����ݣ�ֱ�������ļ�β */
	while (User::NOERROR == u.u_error && u.u_IOParam.m_Count != 0)
	{
		lbn = bn = u.u_IOParam.m_Offset / Inode::BLOCK_SIZE;	/* addr����һ�� */
		offset = u.u_IOParam.m_Offset % Inode::BLOCK_SIZE;		/* ��addr��Ӧ���е�ƫ���� */

		/* ���͵��û������ֽ�������ȡ�������ʣ���ֽ����뵱ǰ�ַ�������Ч�ֽ�����Сֵ */
		nbytes = Utility::Min(Inode::BLOCK_SIZE - offset /* ������Ч�ֽ��� */, u.u_IOParam.m_Count);

		int remain = this->i_size - u.u_IOParam.m_Offset;
		/* ����Ѷ��������ļ���β */
		if (remain <= 0)
		{
			return;
		}
		/* ���͵��ֽ�������ȡ����ʣ���ļ��ĳ��� */
		nbytes = Utility::Min(nbytes, remain);

		/* ���߼����lbnת���������̿��bn */
		if ((bn = this->Bmap(lbn)) == 0)
		{
			return;
		}
		/* ��������Ķ�ȡ */
		pBuf = bufMgr.ReadB(bn);

		/* ��¼�����ȡ�ַ�����߼���� */
		this->i_lastr = lbn;

		/* ������������ʼ��λ�� */
		unsigned char* start = pBuf->b_addr + offset;

		/* ������: �ӻ������������û�Ŀ����
		Utility::ByteCopy(start, u.u_IOParam.m_Base, nbytes);

		/* �ô����ֽ���nbytes���¶�дλ�� */
		u.u_IOParam.m_Base += nbytes;
		u.u_IOParam.m_Offset += nbytes;
		u.u_IOParam.m_Count -= nbytes;

		/* ʹ���껺�棬�ͷŸ���Դ */
		bufMgr.RelseB(pBuf);
	}
}

void Inode::WriteI()
{
	int lbn;	/* �ļ��߼���� */
	int bn;		/* lbn��Ӧ�������̿�� */
	int offset;	/* ��ǰ�ַ�������ʼ����λ�� */
	int nbytes;	/* �����ֽ����� */
	Buf* pBuf;
	User& u = g_User;
	BufferManager& bufMgr = g_BufferManager;

	/* ����Inode�����ʱ�־λ */
	this->i_flag |= (Inode::IACC | Inode::IUPD);

	if (0 == u.u_IOParam.m_Count)
	{
		/* ��Ҫ���ֽ���Ϊ�㣬�򷵻� */
		return;
	}

	while (User::NOERROR == u.u_error && u.u_IOParam.m_Count != 0)
	{
		lbn = u.u_IOParam.m_Offset / Inode::BLOCK_SIZE;
		offset = u.u_IOParam.m_Offset % Inode::BLOCK_SIZE;
		nbytes = Utility::Min(Inode::BLOCK_SIZE - offset, u.u_IOParam.m_Count);
		/* ���߼����lbnת���������̿��bn */
		if ((bn = this->Bmap(lbn)) == 0)
		{
			return;
		}

		if (Inode::BLOCK_SIZE == nbytes)
		{
			/* ���д������������һ���ַ��飬��Ϊ����仺�� */
			pBuf = bufMgr.GetBlk(bn);
		}
		else
		{
			/* д�����ݲ���һ���ַ��飬�ȶ���д���������ַ����Ա�������Ҫ��д�����ݣ� */
			pBuf = bufMgr.ReadB(bn);
		}

		/* ���������ݵ���ʼдλ�� */
		unsigned char* start = pBuf->b_addr + offset;

		/* д����: ���û�Ŀ�����������ݵ������� */
		Utility::ByteCopy(u.u_IOParam.m_Base, start, nbytes);

		/* �ô����ֽ���nbytes���¶�дλ�� */
		u.u_IOParam.m_Base += nbytes;
		u.u_IOParam.m_Offset += nbytes;
		u.u_IOParam.m_Count -= nbytes;

		if (u.u_error != User::NOERROR)	/* д�����г��� */
		{
			bufMgr.RelseB(pBuf);
		}
		else if ((u.u_IOParam.m_Offset % Inode::BLOCK_SIZE) == 0)	/* ���д��һ���ַ��� */
		{
			/* ���첽��ʽ���ַ���д����̣����̲���ȴ�I/O�������������Լ�������ִ�� */
			bufMgr.AWriteB(pBuf);
		}
		else /* ���������δд�� */
		{
			/* ��������Ϊ�ӳ�д�������ڽ���I/O�������ַ�������������� */
			bufMgr.DWriteB(pBuf);
		}

		/* ��ͨ�ļ��������� */
		if (this->i_size < u.u_IOParam.m_Offset)
		{
			this->i_size = u.u_IOParam.m_Offset;
		}

		/*
		 * ֮ǰ�����ж��̿��ܵ��½����л����ڽ���˯���ڼ䵱ǰ�ڴ�Inode����
		 * ��ͬ�������Inode���ڴ���Ҫ�������ø��±�־λ��
		 * ����û�б�Ҫѽ����ʹwriteϵͳ����û��������iput����i_count����0֮��ŻὫ�ڴ�i�ڵ�ͬ���ش��̡�������
		 * �ļ�û��close֮ǰ�ǲ��ᷢ���ġ�
		 * ���ǵ�ϵͳ��writeϵͳ���������͸������ܳ�����������ˡ�
		 * ��������ȥ����
		 */
		this->i_flag |= Inode::IUPD;
	}
}

int Inode::Bmap(int lbn)
{
	Buf* pFirstBuf;
	Buf* pSecondBuf;
	int phyBlkno;	/* ת����������̿�� */
	int* iTable;	/* ���ڷ��������̿���һ�μ�ӡ����μ�������� */
	int index;
	User& u = g_User;
	BufferManager& bufMgr = g_BufferManager;
	FileSystem& fileSys = g_FileSystem;

	/*
	 * Unix V6++���ļ������ṹ��(С�͡����ͺ;����ļ�)
	 * (1) i_addr[0] - i_addr[5]Ϊֱ���������ļ����ȷ�Χ��0 - 6���̿飻
	 *
	 * (2) i_addr[6] - i_addr[7]���һ�μ�����������ڴ��̿�ţ�ÿ���̿�
	 * �ϴ��128���ļ������̿�ţ������ļ����ȷ�Χ��7 - (128 * 2 + 6)���̿飻
	 *
	 * (3) i_addr[8] - i_addr[9]��Ŷ��μ�����������ڴ��̿�ţ�ÿ�����μ��
	 * �������¼128��һ�μ�����������ڴ��̿�ţ������ļ����ȷ�Χ��
	 * (128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
	 */

	/* �������ļ�ϵͳ������Ĵ�С */
	if (lbn >= Inode::HUGE_FILE_BLOCK)
	{
		u.u_error = User::EFBIG;
		return 0;
	}

	if (lbn < 6)		/* �����С���ļ����ӻ���������i_addr[0-5]�л�������̿�ż��� */
	{
		phyBlkno = this->i_addr[lbn];

		/*
		 * ������߼���Ż�û����Ӧ�������̿����֮��Ӧ�������һ������顣
		 * ��ͨ�������ڶ��ļ���д�룬��д��λ�ó����ļ���С�����Ե�ǰ
		 * �ļ���������д�룬����Ҫ�������Ĵ��̿飬��Ϊ֮�����߼����
		 * �������̿��֮���ӳ�䡣
		 */
		if (phyBlkno == 0 && (pFirstBuf = fileSys.AllocB()) != NULL)
		{
			/*
			 * ��������Ϊ�ӳ�д��ʽ���������Լ���ϵͳ��I/O������
			 */
			bufMgr.DWriteB(pFirstBuf);
			phyBlkno = pFirstBuf->b_blkno;

			/* ���߼����lbnӳ�䵽�����̿��phyBlkno */
			this->i_addr[lbn] = phyBlkno;
			this->i_flag |= Inode::IUPD;
		}
		return phyBlkno;
	}
	else	/* lbn >= 6 ���͡������ļ� */
	{
		/* �����߼����lbn��Ӧi_addr[]�е����� */

		if (lbn < Inode::LARGE_FILE_BLOCK)	/* �����ļ�: ���Ƚ���7 - (128 * 2 + 6)���̿�֮�� */
		{
			index = (lbn - Inode::SMALL_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK + 6;
		}
		else	/* �����ļ�: ���Ƚ���263 - (128 * 128 * 2 + 128 * 2 + 6)���̿�֮�� */
		{
			index = (lbn - Inode::LARGE_FILE_BLOCK) / (Inode::ADDRESS_PER_INDEX_BLOCK * Inode::ADDRESS_PER_INDEX_BLOCK) + 8;
		}

		phyBlkno = this->i_addr[index];
		/* ������Ϊ�㣬���ʾ��������Ӧ�ļ��������� */
		if (0 == phyBlkno)
		{
			this->i_flag |= Inode::IUPD;
			/* ����һ�����̿��ż�������� */
			if ((pFirstBuf = fileSys.AllocB()) == NULL)
			{
				return 0;	/* ����ʧ�� */
			}
			/* i_addr[index]�м�¼���������������̿�� */
			this->i_addr[index] = pFirstBuf->b_blkno;
		}
		else
		{
			/* �����洢�����������ַ��� */
			pFirstBuf = bufMgr.ReadB(phyBlkno);
		}
		/* ��ȡ��������ַ */
		iTable = (int*)pFirstBuf->b_addr;

		/* ����8����Ҫͨ������ȡ�õ�Buf�������ٴ�ִ��һ���������� */
		if (index >= 8)	/* ASSERT: 8 <= index <= 9 */
		{
			/* ���ھ����ļ��������pFirstBuf���Ƕ��μ����������������߼���ţ����ɶ��μ���������ҵ�һ�μ�������� */
			index = ((lbn - Inode::LARGE_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;

			/* iTableָ�򻺴��еĶ��μ������������Ϊ�㣬������һ�μ�������� */
			phyBlkno = iTable[index];
			if (0 == phyBlkno)
			{
				if ((pSecondBuf = fileSys.AllocB()) == NULL)
				{
					/* ����һ�μ����������̿�ʧ�ܣ��ͷŻ����еĶ��μ��������Ȼ�󷵻� */
					bufMgr.RelseB(pFirstBuf);
					return 0;
				}
				/* ���·����һ�μ����������̿�ţ�������μ����������Ӧ�� */
				iTable[index] = pSecondBuf->b_blkno;
				/* �����ĺ�Ķ��μ���������ӳ�д��ʽ��������� */
				bufMgr.DWriteB(pFirstBuf);
			}
			else
			{
				/* �ͷŶ��μ��������ռ�õĻ��棬������һ�μ�������� */
				bufMgr.RelseB(pFirstBuf);
				pSecondBuf = bufMgr.ReadB(phyBlkno);
			}

			pFirstBuf = pSecondBuf;
			/* ��iTableָ��һ�μ�������� */
			iTable = (int*)pSecondBuf->b_addr;
		}

		/* �����߼����lbn����λ��һ�μ���������еı������index */
		if (lbn < Inode::LARGE_FILE_BLOCK)
		{
			/* �����ļ� */
			index = (lbn - Inode::SMALL_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;
		}
		else
		{
			/* �����ļ� */
			index = (lbn - Inode::LARGE_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;
		}

		if ((phyBlkno = iTable[index]) == 0 && (pSecondBuf = fileSys.AllocB()) != NULL)
		{
			/* �����䵽���ļ������̿�ŵǼ���һ�μ���������� */
			phyBlkno = pSecondBuf->b_blkno;
			iTable[index] = phyBlkno;
			/* �������̿顢���ĺ��һ�μ�����������ӳ�д��ʽ��������� */
			bufMgr.DWriteB(pSecondBuf);
			bufMgr.DWriteB(pFirstBuf);
		}
		else
		{
			/* �ͷ�һ�μ��������ռ�û��� */
			bufMgr.RelseB(pFirstBuf);
		}
		return phyBlkno;
	}
}

void Inode::UpdateI(int time)
{
	Buf* pBuf;
	DiskInode dInode;
	FileSystem& filesys = g_FileSystem;
	BufferManager& bufMgr = g_BufferManager;

	/* ��IUPD��IACC��־֮һ�����ã�����Ҫ������ӦDiskInode
	 * Ŀ¼����������������;����Ŀ¼�ļ���IACC��IUPD��־ */
	if ((this->i_flag & (Inode::IUPD | Inode::IACC)) != 0)
	{
		/* ���ص�ע�ͣ��ڻ�������ҵ�������i�ڵ㣨this->i_number���Ļ����
		 * ����һ�������Ļ���飬���δ����е�Bwrite()�ڽ������д�ش��̺���ͷŸû���顣
		 * ���ô�Ÿ�DiskInode���ַ�����뻺���� */
		pBuf = bufMgr.ReadB(FileSystem::INODE_ZONE_START_SECTOR + this->i_number / FileSystem::INODE_NUMBER_PER_SECTOR);

		/* ���ڴ�Inode�����е���Ϣ���Ƶ�dInode�У�Ȼ��dInode���ǻ����оɵ����Inode */
		dInode.d_mode = this->i_mode;
		dInode.d_nlink = this->i_nlink;
		dInode.d_uid = this->i_uid;
		dInode.d_gid = this->i_gid;
		dInode.d_size = this->i_size;
		for (int i = 0; i < 10; i++)
		{
			dInode.d_addr[i] = this->i_addr[i];
		}
		if (this->i_flag & Inode::IACC)
		{
			/* ����������ʱ�� */
			dInode.d_atime = time;
		}
		if (this->i_flag & Inode::IUPD)
		{
			/* ����������ʱ�� */
			dInode.d_mtime = time;
		}

		/* ��pָ�򻺴����о����Inode��ƫ��λ�� */
		unsigned char* p = pBuf->b_addr + (this->i_number % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
		DiskInode* pNode = &dInode;

		/* ��dInode�е������ݸ��ǻ����еľ����Inode */
		Utility::DWordCopy((int*)pNode, (int*)p, sizeof(DiskInode) / sizeof(int));

		/* ������д�������̣��ﵽ���¾����Inode��Ŀ�� */
		bufMgr.WriteB(pBuf);
	}
}

/*һ����ͬ  ��־λ*/
void Inode::TruncI()
{
	/* ���ɴ��̸��ٻ����ȡ���һ�μ�ӡ����μ��������Ĵ��̿� */
	BufferManager& bm = g_BufferManager;
	/* ��ȡg_FileSystem��������ã�ִ���ͷŴ��̿�Ĳ��� */
	FileSystem& filesys = g_FileSystem;

	/* ����FILO��ʽ�ͷţ��Ծ���ʹ��SuperBlock�м�¼�Ŀ����̿��������
	 *
	 * Unix V6++���ļ������ṹ��(С�͡����ͺ;����ļ�)
	 * (1) i_addr[0] - i_addr[5]Ϊֱ���������ļ����ȷ�Χ��0 - 6���̿飻
	 *
	 * (2) i_addr[6] - i_addr[7]���һ�μ�����������ڴ��̿�ţ�ÿ���̿�
	 * �ϴ��128���ļ������̿�ţ������ļ����ȷ�Χ��7 - (128 * 2 + 6)���̿飻
	 *
	 * (3) i_addr[8] - i_addr[9]��Ŷ��μ�����������ڴ��̿�ţ�ÿ�����μ��
	 * �������¼128��һ�μ�����������ڴ��̿�ţ������ļ����ȷ�Χ��
	 * (128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
	 */
	for (int i = 9; i >= 0; i--)		/* ��i_addr[9]��i_addr[0] */
	{
		/* ���i_addr[]�е�i��������� */
		if (this->i_addr[i] != 0)
		{
			/* �����i_addr[]�е�һ�μ�ӡ����μ�������� */
			if (i >= 6 && i <= 9)
			{
				/* �������������뻺�� */
				Buf* pFirstBuf = bm.ReadB(this->i_addr[i]);
				/* ��ȡ��������ַ */
				int* pFirst = (int*)pFirstBuf->b_addr;

				/* ÿ�ż���������¼ 512/sizeof(int) = 128�����̿�ţ�������ȫ��128�����̿� */
				for (int j = 128 - 1; j >= 0; j--)
				{
					if (pFirst[j] != 0)	/* �������������� */
					{
						/*
						 * ��������μ��������i_addr[8]��i_addr[9]�
						 * ��ô���ַ����¼����128��һ�μ���������ŵĴ��̿��
						 */
						if (i >= 8 && i <= 9)
						{
							Buf* pSecondBuf = bm.ReadB(pFirst[j]);
							int* pSecond = (int*)pSecondBuf->b_addr;

							for (int k = 128 - 1; k >= 0; k--)
							{
								if (pSecond[k] != 0)
								{
									/* �ͷ�ָ���Ĵ��̿� */
									filesys.FreeI(pSecond[k]);
								}
							}
							/* ����ʹ����ϣ��ͷ��Ա㱻��������ʹ�� */
							bm.RelseB(pSecondBuf);
						}
						filesys.FreeI(pFirst[j]);
					}
				}
				bm.RelseB(pFirstBuf);
			}
			/* �ͷ���������ռ�õĴ��̿� */
			filesys.FreeI(this->i_addr[i]);
			/* 0��ʾ����������� */
			this->i_addr[i] = 0;
		}
	}

	/* �̿��ͷ���ϣ��ļ���С���� */
	this->i_size = 0;
	/* ����IUPD��־λ����ʾ���ڴ�Inode��Ҫͬ������Ӧ���Inode */
	this->i_flag |= Inode::IUPD;
	/* ����ļ���־ ��ԭ����RWXRWXRWX����*/
	this->i_mode &= ~(Inode::ILARG & Inode::IRWXU & Inode::IRWXG & Inode::IRWXO);
	this->i_nlink = 1;
}

void Inode::CleanI()
{
	/*
	 * Inode::Clean()�ض�����IAlloc()������·���DiskInode��ԭ�����ݣ�
	 * �����ļ���Ϣ��Clean()�����в�Ӧ�����i_dev, i_number, i_flag, i_count,
	 * ���������ڴ�Inode����DiskInode�����ľ��ļ���Ϣ����Inode�๹�캯����Ҫ
	 * �����ʼ��Ϊ��Чֵ��
	 */

	this->i_mode = 0;
	this->i_nlink = 0;
	this->i_uid = -1;
	this->i_gid = -1;
	this->i_size = 0;
	this->i_lastr = -1;
	for (int i = 0; i < 10; i++)
	{
		this->i_addr[i] = 0;
	}
}

void Inode::CopyI(Buf* bp, int inumber)
{
	DiskInode dInode;
	DiskInode* pNode = &dInode;

	/* ��pָ�򻺴����б��Ϊinumber���Inode��ƫ��λ�ã���ʱ�������Ϣ�Ѿ����뵽ĳ��buf�У���bp�У� */
	unsigned char* p = bp->b_addr + (inumber % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
	/* �����������Inode���ݿ�������ʱ����dInode�У���4�ֽڿ��� */
	Utility::DWordCopy((int*)p, (int*)pNode, sizeof(DiskInode) / sizeof(int));

	/* �����Inode����dInode����Ϣ���Ƶ��ڴ�Inode�� */
	this->i_mode = dInode.d_mode;
	this->i_nlink = dInode.d_nlink;
	this->i_uid = dInode.d_uid;
	this->i_gid = dInode.d_gid;
	this->i_size = dInode.d_size;
	for (int i = 0; i < 10; i++)
	{
		this->i_addr[i] = dInode.d_addr[i];
	}
}


/*============================class DiskInode=================================*/

DiskInode::DiskInode()
{
	/*
	 * ���DiskInodeû�й��캯�����ᷢ�����½��Ѳ���Ĵ���
	 * DiskInode��Ϊ�ֲ�����ռ�ݺ���Stack Frame�е��ڴ�ռ䣬����
	 * ��οռ�û�б���ȷ��ʼ�����Ծɱ�������ǰջ���ݣ����ڲ�����
	 * DiskInode�����ֶζ��ᱻ���£���DiskInodeд�ص�������ʱ������
	 * ����ǰջ����һͬд�أ�����д�ؽ������Ī����������ݡ�
	 */
	this->d_mode = 0;
	this->d_nlink = 0;
	this->d_uid = -1;
	this->d_gid = -1;
	this->d_size = 0;
	for (int i = 0; i < 10; i++)
	{
		this->d_addr[i] = 0;
	}
	this->d_atime = 0;
	this->d_mtime = 0;
}

DiskInode::~DiskInode()
{
	//nothing to do here
}
	