#ifndef _TREE_ARRAY_MNG_H_
#define _TREE_ARRAY_MNG_H_
//该类是管理多个TreeArray的类,也是提供检索结果的类
#include "isserver/TreeArray.h"
#include "isserver/XHParaCategory.h"
#include "isserver/SmallStruct.h"
#include "isserver/TopResult.h"
#include <string>
#include <map>
#include <queue>

using namespace std;

typedef vector<pair<const XHRbNode*, CTreeArray*> > VEC_MatchedNode;

class CTreeArrayMng {
public:
	CTreeArrayMng();
	static CTreeArrayMng & GetInstance();
	bool SearchResult(map<const string, string> & mapKeyValue, map<const string, CTMInfo *> mapTreeInfo, string & strResult);
private: //查询系列
	//string FindMatchNode();
	bool SearchCategory(vector<CTreeArray*> &vecTreeArray, XHExePara* pxhEPara, TopResult & topResult);
	bool FindMatchNode(vector<CTreeArray*> &vecTreeArray, XHExePara* pxhEPara, VEC_MatchedNode & vecMatchedNode, TopResult & topResult);

	const XHRbNode *FindNodeWithCase(CTreeBase *pTree, const string & strKey, const int iPreAllSize, const XHRbNode * &pNode, const int iType);
	bool CollectResult(const int iTreeType, VEC_MatchedNode& vecMatchedNodes, const int iTotalLen, TopResult & topResult, const bool bSameLen, int iKeyLen);
	bool CollectNode(CTreeArray *pTreeArray, const int iTreeType, const XHRbNode* pNode, TopResult & topResult, int iKeyLen);
	bool FindMatchNode_Common(vector<CTreeArray*> &vecTreeArray, const int iTreeType, const string &strPre, const string &strKey, const int iPreNullSize, VEC_MatchedNode &vecMatchedNode, const bool bSameLen);
	bool FindMatchNode_NineGrid(vector<CTreeArray*> &vecTreeArray, const int iTreeType, const string &strPre, const string &strKey, const int iPreNullSize, VEC_MatchedNode &vecMatchedNode, const bool bSameLen);
	bool FindMatchNode_MixedSS(vector<CTreeArray*> &vecTreeArray, const int iTreeType, const string &strPre, const string &strKey, const int iPreNullSize, VEC_MatchedNode &vecMatchedNode, TopResult & topResult, const bool bSameLen);
	bool FindMatchNode_MixedCS(vector<CTreeArray*> &vecTreeArray, const int iTreeType, const string &strPre, const string &strKey, const int iPreNullSize, VEC_MatchedNode &vecMatchedNode, TopResult & topResult, const bool bSameLen);
private:
	void Init(); //初始化
	int GetSpellTreeFromSimple(const int iTree);

private: //变量
	bool  m_bNoDotSearch; //是否支持dot查询,如果不支持,则spell树中不区分大小写,暂时认为都支持
	SysFlagForCategory m_stSysFlag;
	string m_strPeerAddTaskName;
};

#endif
