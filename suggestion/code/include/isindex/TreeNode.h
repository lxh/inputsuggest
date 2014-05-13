
#ifndef _COMPRO_TREENODE_H
#define _COMPRO_TREENODE_H

#include <stdio.h>
#include "isindex/LeafNode.h"

//计算所有的孩子结点中最大的数值
#define COMPUTER_TOTAL_SUN 1

class CTreeNode
{
public:
	CTreeNode();
	virtual ~CTreeNode();

	/****************************************** Clear ***********************************************
	功能：置CTreeNode节点为空节点
	返回值：
	参数：
	程序员：
	创建日期：
	最后修改日期：
	备注：
	************************************************************************************************/
	void clear();

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
	void set_node(short sbyte_index, char cbyte, CTreeNode *pmiddle_node, CTreeNode *pparent_node, unsigned char ucolor, CLeafNode *pitem);

//protected:
	friend class CTSTPat;
	//
	
	/////////////////////////////////////////////
    //add by lxh
	bool bHavePeer;             //有与自己成对的字母(当前的cByte为大写时,也存在一个小写的该字母)
	/////////////////////////////////////////////

	char cbyte_;				//不同字节内容
	//
	unsigned char ucolor_;   //但是在处理拼音树的时候,也会存储一些属性信息,树建立好之后颜色就没有用了
	unsigned short usWeight;    //权重
	short sbyte_index_;         //不同字节的位置
	//
    unsigned int m_uiMemPos;
	CTreeNode *pleft_;
	CTreeNode *pmiddle_;
	CTreeNode *pright_;
	//
	CLeafNode *pitem_;
	//
	CTreeNode *pparent_;
};

#endif 
