
#ifndef _COMPRO_TSTPAT_H
#define _COMPRO_TSTPAT_H

#define BLACK	1
#define RED		2

#include "isindex/TreeNode.h"
#include "util/UnitMemoryManager.h"

/*
	��ϸ��������middle������; �����ռ�������ͷ�������ӽڵ�;  traversing
*/

class CTSTPat
{
public:
	int alloc_nums();
	CTSTPat();
	virtual ~CTSTPat();

	int init(int nmax_tree_block_num, int ntree_block_unit_num);

	/***************************************** insert ***********************************************
	���ܣ����������������
	����ֵ���ɹ�����0��ʧ�ܷ���-1(������ͬ�ڵ�)���ڴ治�㷵��1
	������
		CTreeNode			*&proot		in		ǰ׺����
		CLeafNode			*pitem		in		������ָ��
	����Ա��
	�������ڣ�
	����޸����ڣ�
	��ע��
	************************************************************************************************/
	long insert(CTreeNode *&proot, CLeafNode *pitem);

	/****************************************** search **********************************************
	���ܣ���ȷ����pitem����
	����ֵ���ɹ����ض�ӦҶ�ӽڵ㣬ʧ�ܷ���NULL
	������
		CTreeNode		*proot		in		ǰ׺����
		CLeafNode		*pitem		in		��������
	����Ա��
	�������ڣ�
	����޸����ڣ�
	��ע��
	************************************************************************************************/
	CLeafNode *search(CTreeNode *proot, CLeafNode *pitem);

	CLeafNode* remove(CTreeNode *&proot, CLeafNode *pitem);

	void clear();

private:
	/**************************************** left_rotate *******************************************
	���ܣ��ڵ���������
	����ֵ��
	������
		CTreeNode		*&proot		out		������ָ������
		CTreeNode		*pnode		in		��ת�ڵ�
	����Ա��
	�������ڣ�
	����޸����ڣ�
	��ע��
	************************************************************************************************/
	inline void left_rotate(CTreeNode *&proot, CTreeNode *pnode);

	/**************************************** right_rotate ******************************************
	���ܣ��ڵ���������
	����ֵ��
	������
		CTreeNode		*&proot		out		������ָ������
		CTreeNode		*pnode		in		��ת�ڵ�
	����Ա��
	�������ڣ�
	����޸����ڣ�
	��ע��
	************************************************************************************************/
	inline void right_rotate(CTreeNode *&proot, CTreeNode *pnode);

	/**************************************** add_fixup *********************************************
	���ܣ���ͬ���ڲ��ڵ��������ӽڵ�ĵ�������
	����ֵ���ɹ�����0,ʧ�ܷ���-1
	������
		CTreeNode		*&proot		out		������ָ������
		CTreeNode		*pnode		in		���ӽڵ�
	����Ա��
	�������ڣ�
	����޸����ڣ�
	��ע��
	************************************************************************************************/
	inline int add_fixup(CTreeNode *&proot, CTreeNode *pnode);

	inline int del_fixup(CTreeNode *&proot, CTreeNode *pnode);

	inline CTreeNode *tree_min(CTreeNode *proot);

protected:
	CUnitMemoryManager *ptree_node_manager_;	//���ڵ��ڴ������

};

#endif 
