
#ifndef _COMPRO_TSTPAT_H
#define _COMPRO_TSTPAT_H

#define BLACK	1
#define RED		2

#include "isindex/TreeNode.h"
#include "util/UnitMemoryManager.h"

/*
	仔细分析增加middle的作用; 便于收集以他开头的所有子节点;  traversing
*/

class CTSTPat
{
public:
	int alloc_nums();
	CTSTPat();
	virtual ~CTSTPat();

	int init(int nmax_tree_block_num, int ntree_block_unit_num);

	/***************************************** insert ***********************************************
	功能：将数据项插入树中
	返回值：成功返回0，失败返回-1(存在相同节点)，内存不足返回1
	参数：
		CTreeNode			*&proot		in		前缀树根
		CLeafNode			*pitem		in		数据项指针
	程序员：
	创建日期：
	最后修改日期：
	备注：
	************************************************************************************************/
	long insert(CTreeNode *&proot, CLeafNode *pitem);

	/****************************************** search **********************************************
	功能：精确检索pitem数据
	返回值：成功返回对应叶子节点，失败返回NULL
	参数：
		CTreeNode		*proot		in		前缀树根
		CLeafNode		*pitem		in		检索数据
	程序员：
	创建日期：
	最后修改日期：
	备注：
	************************************************************************************************/
	CLeafNode *search(CTreeNode *proot, CLeafNode *pitem);

	CLeafNode* remove(CTreeNode *&proot, CLeafNode *pitem);

	void clear();

private:
	/**************************************** left_rotate *******************************************
	功能：节点左旋操作
	返回值：
	参数：
		CTreeNode		*&proot		out		层树根指针引用
		CTreeNode		*pnode		in		旋转节点
	程序员：
	创建日期：
	最后修改日期：
	备注：
	************************************************************************************************/
	inline void left_rotate(CTreeNode *&proot, CTreeNode *pnode);

	/**************************************** right_rotate ******************************************
	功能：节点右旋操作
	返回值：
	参数：
		CTreeNode		*&proot		out		层树根指针引用
		CTreeNode		*pnode		in		旋转节点
	程序员：
	创建日期：
	最后修改日期：
	备注：
	************************************************************************************************/
	inline void right_rotate(CTreeNode *&proot, CTreeNode *pnode);

	/**************************************** add_fixup *********************************************
	功能：对同层内部节点作层增加节点的调整操作
	返回值：成功返回0,失败返回-1
	参数：
		CTreeNode		*&proot		out		层树根指针引用
		CTreeNode		*pnode		in		增加节点
	程序员：
	创建日期：
	最后修改日期：
	备注：
	************************************************************************************************/
	inline int add_fixup(CTreeNode *&proot, CTreeNode *pnode);

	inline int del_fixup(CTreeNode *&proot, CTreeNode *pnode);

	inline CTreeNode *tree_min(CTreeNode *proot);

protected:
	CUnitMemoryManager *ptree_node_manager_;	//树节点内存管理器

};

#endif 
