#include "isindex/LeafNode.h"

CLeafNode::CLeafNode():pkey_(NULL),ukey_len_(0)
{
}

CLeafNode::~CLeafNode()
{
}

/***************************************** set_key **********************************************
功能：设置数据项的关键字
返回值：成功返回true,失败返回false
参数：
	char			*pkey		in		关键字字串首指针
	unsigned char	ulen		in		关键字字节长度
程序员：
创建日期：
最后修改日期：
备注：判别pKey和uLen有意义,失败不改变数据项原有内容
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
功能：寻找两个数据项关键字第一个不同的Byte
返回值：成功返回不同Byte的序号，失败返回-1
参数：
	CTreeNode			*pitem			in		另一个数据项
	short				sstart_offset	in		起始偏移
程序员：
创建日期：
最后修改日期：
备注：两个数据项都是空的返回-1，数据项相等返回-1，一个数据项是空返回0
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
功能：获得数据项中关键字指定字节的数据
返回值：成功返回字节数据,失败返回0
参数：
	long		lseq_num		in		指定关键字字节序号
程序员：
创建日期：
最后修改日期：
备注：指定关键字字节序号超出关键字长返回0
************************************************************************************************/
unsigned char CLeafNode::get_byte(long lseq_num)
{
	if(lseq_num >= 0 && lseq_num < ukey_len_)
		return pkey_[lseq_num];
	else
		return 0;
}

/******************************************* == *************************************************
功能：判断数据项中关键字的相等逻辑关系
返回值：相等返回true,不相等返回false
参数：
	const CLeafNode		&item		in		右参数
程序员：
创建日期：
最后修改日期：
备注：任何一个数据项为空返回false
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
功能：获得关键字字符串
返回值：返回关键字首指针
参数：
	long		&llen		out			输出关键字长度
程序员：
创建日期：
最后修改日期：
备注：
************************************************************************************************/
char *CLeafNode::key(long &llen)
{
	llen = ukey_len_;
	return pkey_;
}

/******************************************** < *************************************************
功能：判断数据项中关键字的小于逻辑关系
返回值：小于item返回true,大于等于Item返回false
参数：
	const CLeafNode		&item		in		右参数
程序员：
创建日期：
最后修改日期：
备注：
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
功能：判断数据项是否为空，以关键字是否为空为准
返回值：为空返回true,不空返回false
参数：
程序员：
创建日期：
最后修改日期：
备注：
************************************************************************************************/
bool CLeafNode::is_null()
{
	if(ukey_len_)
		return false;
	else
		return true;
}

/*************************************** is_single_byte *****************************************
功能：判断数据项是否为单字节串
返回值：单字节串返回true,否则返回false
参数：
程序员：
创建日期：
最后修改日期：
备注：
************************************************************************************************/
bool CLeafNode::is_single_byte()
{
	if(1 == ukey_len_)
		return true;
	else
		return false;
}

/**************************************** get_head_offset ***************************************
功能：获得串首1或2字节的编码值
返回值：成功返回编码值,空数据项返回0
参数：
程序员：
创建日期：
最后修改日期：
备注：
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
