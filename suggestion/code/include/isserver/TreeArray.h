#ifndef _TREE_ARRAY_H_
#define _TREE_ARRAY_H_
#include <string>
#include "util/DumpPickIndex.h"
#include "util/SysConfigSet.h"
#include "isserver/XHTree.h"

using namespace std;

class CTreeArray {
public:
	CTreeArray(const string & strTaskName);
	~CTreeArray();
	CTreeBase * GetTree(int iTreeType);
	string PrintTree(const string& strPrefix, const string &strSearchKey, const unsigned int uiDepth, const unsigned int uiTreeType);
	unsigned int GetTaskUseSize() {
		if(m_pTreeData == NULL) return 0;
		return ((unsigned int*)m_pTreeData)[0];
	};

private: //��ʼ�����
	void Init();
	CTreeBase *NewTree(int iType);
	const string Debug_PrintTree(CTreeBase * pTree, const unsigned int uiMaxDepth, const int iTreeType, const XHRbNode * clsTreeNode, const int iDepth = 0, const string & strType = "root");

private: //��ѯ���
private:
	string m_strTaskName;
	char * m_pTreeData;          //�������е�����������ָ��
	IndexFiles * m_pIndexFiles;
	CTreeBase  * m_pTrees[DICT_MAX_SIZE];
};

#endif
