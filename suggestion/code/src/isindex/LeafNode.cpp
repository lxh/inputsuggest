#include "isindex/LeafNode.h"

CLeafNode::CLeafNode():pkey_(NULL),ukey_len_(0)
{
}

CLeafNode::~CLeafNode()
{
}

/***************************************** set_key **********************************************
���ܣ�����������Ĺؼ���
����ֵ���ɹ�����true,ʧ�ܷ���false
������
	char			*pkey		in		�ؼ����ִ���ָ��
	unsigned char	ulen		in		�ؼ����ֽڳ���
����Ա��
�������ڣ�
����޸����ڣ�
��ע���б�pKey��uLen������,ʧ�ܲ��ı�������ԭ������
************************************************************************************************/
bool CLeafNode::set_key(char *pkey, unsigned char ulen)
{
	if((pkey && ulen > 0) || (NULL == pkey && 0 == ulen))
	{
		pkey_ = pkey;
		ukey_len_ = ulen;

		return true;
	}
	else
	{
		return false;
	}
}

bool CLeafNode::setId(int id)
{
	elemid = id;
	return true;
}

/********************************** get_first_different_byte ************************************
���ܣ�Ѱ������������ؼ��ֵ�һ����ͬ��Byte
����ֵ���ɹ����ز�ͬByte����ţ�ʧ�ܷ���-1
������
	CTreeNode			*pitem			in		��һ��������
	short				sstart_offset	in		��ʼƫ��
����Ա��
�������ڣ�
����޸����ڣ�
��ע������������ǿյķ���-1����������ȷ���-1��һ���������ǿշ���0
************************************************************************************************/
long CLeafNode::get_first_different_byte(CLeafNode *pitem, short sstart_offset)
{
	short i = sstart_offset;

	if(ukey_len_ < pitem->ukey_len_)
	{
		while(i < ukey_len_ && pkey_[i] == pitem->pkey_[i])
			i++;
	}
	else if(ukey_len_ > pitem->ukey_len_)
	{
		while(i <pitem->ukey_len_ && pkey_[i] == pitem->pkey_[i])
			i++;
	}
	else
	{
		while(i < ukey_len_ && pkey_[i] == pitem->pkey_[i])
			i++;
		if(i == ukey_len_)
			return -1;
	}

	return i;
}

/**************************************** get_byte **********************************************
���ܣ�����������йؼ���ָ���ֽڵ�����
����ֵ���ɹ������ֽ�����,ʧ�ܷ���0
������
	long		lseq_num		in		ָ���ؼ����ֽ����
����Ա��
�������ڣ�
����޸����ڣ�
��ע��ָ���ؼ����ֽ���ų����ؼ��ֳ�����0
************************************************************************************************/
unsigned char CLeafNode::get_byte(long lseq_num)
{
	if(lseq_num >= 0 && lseq_num < ukey_len_)
		return pkey_[lseq_num];
	else
		return 0;
}

/******************************************* == *************************************************
���ܣ��ж��������йؼ��ֵ�����߼���ϵ
����ֵ����ȷ���true,����ȷ���false
������
	const CLeafNode		&item		in		�Ҳ���
����Ա��
�������ڣ�
����޸����ڣ�
��ע���κ�һ��������Ϊ�շ���false
************************************************************************************************/
bool CLeafNode::operator ==(const CLeafNode &item) const
{
	if(ukey_len_ == item.ukey_len_ && ukey_len_ != 0)
	{
		if(memcmp(pkey_, item.pkey_, ukey_len_))
			return false;
		else
			return true;
	}
	else
		return false;
}

bool CLeafNode::operator >=(const CLeafNode &item) const
{
	if(ukey_len_ >= item.ukey_len_ && item.ukey_len_)
	{
		if(memcmp(pkey_, item.pkey_, item.ukey_len_))
			return false;
		else
			return true;
	}
	else
		return false;
}

/******************************************* key ************************************************
���ܣ���ùؼ����ַ���
����ֵ�����عؼ�����ָ��
������
	long		&llen		out			����ؼ��ֳ���
����Ա��
�������ڣ�
����޸����ڣ�
��ע��
************************************************************************************************/
char *CLeafNode::key(long &llen)
{
	llen = ukey_len_;
	return pkey_;
}

/******************************************** < *************************************************
���ܣ��ж��������йؼ��ֵ�С���߼���ϵ
����ֵ��С��item����true,���ڵ���Item����false
������
	const CLeafNode		&item		in		�Ҳ���
����Ա��
�������ڣ�
����޸����ڣ�
��ע��
************************************************************************************************/
bool CLeafNode::operator <(const CLeafNode &item) const
{
	if(ukey_len_ < item.ukey_len_)
	{
		if(ukey_len_ == 0)
			return true;
		else if(memcmp(pkey_, item.pkey_, ukey_len_) <= 0)
			return true;
		else
			return false;
	}
	else if(ukey_len_ < item.ukey_len_)
	{
		if(item.ukey_len_ == 0)
			return false;
		else if(memcmp(pkey_, item.pkey_, item.ukey_len_) < 0)
			return true;
		else
			return false;
	}
	else
	{
		if(ukey_len_ == 0)
			return false;
		else if(memcmp(pkey_, item.pkey_, ukey_len_) < 0)
			return true;
		else
			return false;
	}
}

/***************************************** is_null **********************************************
���ܣ��ж��������Ƿ�Ϊ�գ��Թؼ����Ƿ�Ϊ��Ϊ׼
����ֵ��Ϊ�շ���true,���շ���false
������
����Ա��
�������ڣ�
����޸����ڣ�
��ע��
************************************************************************************************/
bool CLeafNode::is_null()
{
	if(ukey_len_)
		return false;
	else
		return true;
}

/*************************************** is_single_byte *****************************************
���ܣ��ж��������Ƿ�Ϊ���ֽڴ�
����ֵ�����ֽڴ�����true,���򷵻�false
������
����Ա��
�������ڣ�
����޸����ڣ�
��ע��
************************************************************************************************/
bool CLeafNode::is_single_byte()
{
	if(1 == ukey_len_)
		return true;
	else
		return false;
}

/**************************************** get_head_offset ***************************************
���ܣ���ô���1��2�ֽڵı���ֵ
����ֵ���ɹ����ر���ֵ,���������0
������
����Ա��
�������ڣ�
����޸����ڣ�
��ע��
************************************************************************************************/
unsigned short CLeafNode::get_head_offset()
{
	if(1 == ukey_len_)
		return *(unsigned char*)pkey_;
	else if(1 < ukey_len_)
		return *(unsigned short*)pkey_;
	else
		return 0;
}
