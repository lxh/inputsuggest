
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
功能：置CTreeNode节点为空节点
返回值：
参数：
程序员：
创建日期：
最后修改日期：
备注：
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
功能：填充CTreeNode节点
返回值：
参数：
	short					sbyte_index		in		内部节点相异位序号取值
	char					cbyte			in		内部节点相异位取值
	CTreeNode				*pmiddle_node	in		内部节点中间子节点取值
	CTreeNode				*pparent_node	in		同层二叉树意义下的祖先节点
	unsigned char			ucolor			in		同层红黑树节点着色
	CLeafNode				*pitem			in		数据项指针
程序员：
创建日期：
最后修改日期：
备注：
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
