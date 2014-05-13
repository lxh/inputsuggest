#ifndef _INDEX_TREE_INFO_H_
#define _INDEX_TREE_INFO_H_
#include "isindex/IndexDataInfo.h"
#include "isindex/TreeNode.h"
#include "isindex/TSTPat.h"
#include "isindex/PinyinExtra.h"
//��������������Ҫ��,��һ������,���Ұ���Ҫ���е�һЩת��ת����.
//����ƴ����Ҫ�Ѽ�ƴȫƴ���ֵĻ�ϲ�ѯ��������

//��ƴȫƴ��ϲ�ѯ���ַ�����̳���      
#define CONST_MIXSEARCH_HEAD_LEN_MIN 2  
#define CONST_MIXSEARCH_HEAD_LEN_MAX 128

using namespace std;

class CIndexTreeInfo {
public:
	int Run(const string & strTaskName, CSysConfigSet *pclsSysConfigSet);

private:
	void InitParamter();
	int Process();


private: //�����
	bool BuildTrees(const int iSelectId);
	bool CreateSearchTree(const int iSelectId, CTreeNode * & pTreeNodeRoot, CTSTPat & clsTSTPat, CSimpleToSpell & csToSpell);
	bool MarkHavePeerInfo(CTreeNode * & pTreeNode);
	bool CollectAllBrother(CTreeNode * & pTreeNode, map<const int, map<const char, vector<CTreeNode*> > >& mapT);
	bool WriteTreeNode(const int iSelectId, CTreeNode * & clsTreeNode, CSimpleToSpell & csToSpell);
	bool GetMixSearchContext(CSimpleToSpell &csToSpell, int & iFd, XHRbNode &xhRbNode, CTreeNode * & clsTreeNode, CTreeNode * const & clsNodeParent, unsigned int & uiSSLen, const int iMixSearch);
	bool Do_WriteTreeNode(int &iFdSS, CSimpleToSpell &csToSpell, CTreeNode * const & clsTreeRoot, const unsigned int & uiMemPosCanUse, unsigned int & uiMemPosUsing, unsigned int & uiMemUseTotal, int & iFileFd, CTreeNode * & clsTreeNode, unsigned int & uiSSLen, CTreeNode * const & clsNodeParent, const int iMixSearch);
	int HasMixSearch(const int iSelectId);
	void FreeTreeNodeData(CTreeNode * & clsTreeNode);
	bool GetCompressMapping(const int iSelectId, IdToId * & pstIdToId);
	bool GetChineseDataAndString(const int iSelectId, XHChineseData * & pXHCD, char * &pChineseString);
private:
	CSysConfigSet * m_pSysConfigSet; //�����ļ�����Ϣ                 
	string          m_strTaskName;   //���������                     
	string          m_strDataPath;   //���ݵ�·��                     
	bool            m_bDictNeedCreate[DICT_MAX_SIZE]; //��Ҫ����Ĵʵ�
	int             m_iPrefixLen;
};

#endif
