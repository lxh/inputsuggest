#ifndef _INDEX_TREE_INFO_H_
#define _INDEX_TREE_INFO_H_
#include "isindex/IndexDataInfo.h"
#include "isindex/TreeNode.h"
#include "isindex/TSTPat.h"
#include "isindex/PinyinExtra.h"
//该类有两件事情要做,第一创建树,并且把树要进行的一些转换转换了.
//对于拼音还要把简拼全拼汉字的混合查询给处理了

//简拼全拼混合查询的字符串最短长度      
#define CONST_MIXSEARCH_HEAD_LEN_MIN 2  
#define CONST_MIXSEARCH_HEAD_LEN_MAX 128

using namespace std;

class CIndexTreeInfo {
public:
	int Run(const string & strTaskName, CSysConfigSet *pclsSysConfigSet);

private:
	void InitParamter();
	int Process();


private: //树相关
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
	CSysConfigSet * m_pSysConfigSet; //配置文件的信息                 
	string          m_strTaskName;   //任务的名称                     
	string          m_strDataPath;   //数据的路径                     
	bool            m_bDictNeedCreate[DICT_MAX_SIZE]; //需要处理的词典
	int             m_iPrefixLen;
};

#endif
