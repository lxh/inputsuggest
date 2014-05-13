
#ifndef _COMPRO_TREENODE_H
#define _COMPRO_TREENODE_H

#include <stdio.h>
#include "isindex/LeafNode.h"

//�������еĺ��ӽ����������ֵ
#define COMPUTER_TOTAL_SUN 1

class CTreeNode
{
public:
	CTreeNode();
	virtual ~CTreeNode();

	/****************************************** Clear ***********************************************
	���ܣ���CTreeNode�ڵ�Ϊ�սڵ�
	����ֵ��
	������
	����Ա��
	�������ڣ�
	����޸����ڣ�
	��ע��
	************************************************************************************************/
	void clear();

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
	void set_node(short sbyte_index, char cbyte, CTreeNode *pmiddle_node, CTreeNode *pparent_node, unsigned char ucolor, CLeafNode *pitem);

//protected:
	friend class CTSTPat;
	//
	
	/////////////////////////////////////////////
    //add by lxh
	bool bHavePeer;             //�����Լ��ɶԵ���ĸ(��ǰ��cByteΪ��дʱ,Ҳ����һ��Сд�ĸ���ĸ)
	/////////////////////////////////////////////

	char cbyte_;				//��ͬ�ֽ�����
	//
	unsigned char ucolor_;   //�����ڴ���ƴ������ʱ��,Ҳ��洢һЩ������Ϣ,��������֮����ɫ��û������
	unsigned short usWeight;    //Ȩ��
	short sbyte_index_;         //��ͬ�ֽڵ�λ��
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
