
#include "isindex/TreeNode.h"

CTreeNode::CTreeNode():cbyte_(0),ucolor_(0),sbyte_index_(0),pleft_(NULL),pmiddle_(NULL),pright_(NULL),pitem_(NULL),pparent_(NULL)
{
    m_uiMemPos = 0;
	usWeight = 0;
}

CTreeNode::~CTreeNode()
{
}

/****************************************** Clear ***********************************************
���ܣ���CTreeNode�ڵ�Ϊ�սڵ�
����ֵ��
������
����Ա��
�������ڣ�
����޸����ڣ�
��ע��
************************************************************************************************/
void CTreeNode::clear()
{
    m_uiMemPos = 0;
	usWeight = 0;
	cbyte_ = 0;
	ucolor_ = 0;
	sbyte_index_ = 0;
	pleft_ = NULL;
	pmiddle_ = NULL;
	pright_ = NULL;
	pitem_ = NULL;
	pparent_ = NULL;
}

/****************************************** set_node ********************************************
���ܣ����CTreeNode�ڵ�
����ֵ��
������
	short					sbyte_index		in		�ڲ��ڵ�����λ���ȡֵ
	char					cbyte			in		�ڲ��ڵ�����λȡֵ
	CTreeNode				*pmiddle_node	in		�ڲ��ڵ��м��ӽڵ�ȡֵ
	CTreeNode				*pparent_node	in		ͬ������������µ����Ƚڵ�
	unsigned char			ucolor			in		ͬ�������ڵ���ɫ
	CLeafNode				*pitem			in		������ָ��
����Ա��
�������ڣ�
����޸����ڣ�
��ע��
************************************************************************************************/
void CTreeNode::set_node(short sbyte_index, char cbyte, CTreeNode *pmiddle_node, CTreeNode *pparent_node, unsigned char ucolor, CLeafNode *pitem)
{
	sbyte_index_ = sbyte_index;
	cbyte_ = cbyte;
	pleft_ = NULL;
	pmiddle_ = pmiddle_node;
	pright_ = NULL;
	pparent_ = pparent_node;
	ucolor_ = ucolor;
	pitem_ = pitem;
	usWeight = 0;
    m_uiMemPos = 0;
}
