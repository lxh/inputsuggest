#include "isserver/TreeArray.h"
#include "util/CompressPinyin.h"

CTreeArray::CTreeArray(const string & strTaskName)
{
	m_strTaskName = strTaskName;
	m_pTreeData   = NULL;
	m_pIndexFiles = NULL;
	memset(m_pTrees, 0x00, sizeof(m_pTrees[0]) * DICT_MAX_SIZE);
	Init();
}


CTreeArray::~CTreeArray()
{
	SAFE_DELETE_MUL(m_pTreeData);
	int iLoop;
	for(iLoop = 0; iLoop < DICT_MAX_SIZE; iLoop++) {
		SAFE_DELETE_SIG(m_pTrees[iLoop]);
	}
}

CTreeBase *CTreeArray::GetTree(int iType)
{
	return m_pTrees[iType];
}

//申请新的树
CTreeBase *CTreeArray::NewTree(int iType)
{
	CTreeBase * p;
	switch(iType) {
		case SELECT_TREE_CHINESE          : p = new CTreeChinese        (); return p;
		case SELECT_TREE_SPELL            : p = new CTreeSpell          (); return p;
		case SELECT_TREE_SIMPLE           : p = new CTreeSimple         (); return p;
		case SELECT_TREE_CENTER_CHINESE   : p = new CTreeCenterChinese  (); return p;
		case SELECT_TREE_CENTER_SPELL     : p = new CTreeCenterSpell    (); return p;
		case SELECT_TREE_CENTER_SIMPLE    : p = new CTreeCenterSimple   (); return p;
		case SELECT_TREE_EX_CHINESE       : p = new CTreeExChinese      (); return p;
		case SELECT_TREE_EX_SPELL         : p = new CTreeExSpell        (); return p;
		case SELECT_TREE_EX_SIMPLE        : p = new CTreeExSimple       (); return p;
		case SELECT_TREE_EX_CENTER_CHINESE: p = new CTreeExCenterChinese(); return p;
		case SELECT_TREE_EX_CENTER_SPELL  : p = new CTreeExCenterSpell  (); return p;
		case SELECT_TREE_EX_CENTER_SIMPLE : p = new CTreeExCenterSimple (); return p;
	}
	return NULL;
}

//初始化
void CTreeArray::Init()
{
	string strIndexFile = G_STR(pathdata) + "/" + m_strTaskName + "/" + m_strTaskName;
	m_pTreeData = CDumpPickIndex::Pick(strIndexFile);
	if(!m_pTreeData) {
		printf("error: read index(%s) [%s %d]\n", strIndexFile.c_str(), __FILE__, __LINE__);
		return;
	}
	m_pIndexFiles = (IndexFiles*)m_pTreeData;

	int iLoop;
	char *p = m_pTreeData;
	for(iLoop = 0; iLoop < DICT_MAX_SIZE; iLoop++) {
		if(m_pIndexFiles[iLoop].bSelect) {
			m_pTrees[iLoop] = NewTree(iLoop);
			if(m_pTrees[iLoop]) {
				m_pTrees[iLoop]->m_pzExtraInfo   = p + m_pIndexFiles->uiExtra[iLoop];
				m_pTrees[iLoop]->m_pcStringData  = p + m_pIndexFiles->uiString[iLoop];
				m_pTrees[iLoop]->m_pzTreeWife    = p + m_pIndexFiles->uiNodeData[iLoop];
				m_pTrees[iLoop]->m_pstTreeRoot   = p + m_pIndexFiles->uiTreeNode[iLoop];
				m_pTrees[iLoop]->m_pstSimpleSelf = p + m_pIndexFiles->uiMixSearch[iLoop];
			}
		} else {
			m_pTrees[iLoop] = NULL;
		}
	}
}

//打印树的结构,用于调试使用
string CTreeArray::PrintTree(const string& strPrefix, const string &strSearchKey, const unsigned int uiDepth, const unsigned int uiTreeType)
{
	if(!m_pIndexFiles->bSelect[uiTreeType]) {
		return "the tree is not exist";
	}
	CTreeBase * pTree = GetTree(uiTreeType);
	const XHRbNode  * pTreeRoot = pTree->GetTreeRoot();
	if(!pTreeRoot) {
		return "root is empty";
	}
	const XHRbNode * p = pTreeRoot;
	string strT = strPrefix + strSearchKey;
	while(1) {
		char cCase = strT[p->cPosDif];
		char cNode = p->cByteDif;
		if(cCase < cNode) {
			p  = pTree->GetTreeLeft(p);
		} else if(cCase == cNode) {
			const XHRbNode * pMid = pTree->GetTreeMid(p);
			if(!pMid) {
				const char *pStr = pTree->GetString(p);
				if(strlen(pStr) >= strSearchKey.size() && memcmp(pStr, strSearchKey.c_str(), strSearchKey.size()) == 0) {
					break;
				}
				return "\"result\":\"not found\"";
			} else {
				if(pMid->cPosDif >= strT.size()) {
					const char *pStr = pTree->GetString(p);
					if(strlen(pStr) >= strSearchKey.size() && memcmp(pStr, strSearchKey.c_str(), strSearchKey.size()) == 0) {
						break;
					}
					return "\"result\":\"not found\"";
				} else {
					p = pMid;
					continue;
				}
			}
		} else {
			p  = pTree->GetTreeRight(p);
		}
		if(!p) {
			return "\"result\":\"not found\"";
		}
	}
	string strRet = Debug_PrintTree(pTree, uiDepth, uiTreeType, p);
	strRet = "\"result\":" + strRet;
	return strRet;
}
//打印指定树的形状
const string CTreeArray::Debug_PrintTree(CTreeBase * pTree, const unsigned int uiMaxDepth, const int iTreeType, const XHRbNode * clsTreeNode, const int iDepth, const string & strType)
{
	char szOut[2048];
	if(iDepth >= uiMaxDepth) {
		return "";
	}
	string strResult = "";
	int iMove = iDepth > 128 ? 128 : iDepth;
	//打印头
	memset(szOut, '-', iMove);
	szOut[iMove - 1] = '|';
	strResult += "{";

	iMove = 0;
	snprintf(szOut + iMove, sizeof(szOut) - iMove, "\"new_node\":\"%s\",\n", strType.c_str());
	strResult += szOut;
	memset(szOut, ' ', iMove);
	//unsigned int t = clsTreeNode->cByteDif & 0xff;
	unsigned char t = clsTreeNode->cByteDif;
	if(t >= 'A' && t <= 'Z' || t >= 'a' && t <= 'z' || t >= '0' && t <= '9') {
		snprintf(szOut + iMove, sizeof(szOut) - iMove, "\"cByteDif_char\":\"%c\",\n", clsTreeNode->cByteDif);
	} else {
		snprintf(szOut + iMove, sizeof(szOut) - iMove, "\"cByteDif_int\":\"%d\",\n", clsTreeNode->cByteDif);
	}
	strResult += szOut;
	snprintf(szOut + iMove, sizeof(szOut) - iMove, "\"cPosDif\":%u,\n", clsTreeNode->cPosDif);
	strResult += szOut;
	snprintf(szOut + iMove, sizeof(szOut) - iMove, "\"usWeight\":%u,\n", clsTreeNode->usWeight);
	strResult += szOut;
	snprintf(szOut + iMove, sizeof(szOut) - iMove, "\"simple_spell\":%u,\n", clsTreeNode->usHua & XH_TREE_HUA_INFO_SSELF);
	strResult += szOut;
	if((clsTreeNode->usHua & XH_TREE_HUA_INFO_SSELF) && ALL_SELECT_SIMPLE_TREE(iTreeType)) {
		unsigned int ui = clsTreeNode->UncomposeSpellSelf() & 0x7fffffff;
		char cFrom = ((char *)pTree->GetPMove(pTree->m_pstSimpleSelf, ui))[0];
		char szCompress[1024] = {0};
		string strTail = "";
		snprintf(szOut + iMove, sizeof(szOut) - iMove, "\"ss_data_%d\":[\n", (ui & 0x80000000) ? 1 : 0);
		strResult += szOut;
		const char *pzStr = pTree->GetString(clsTreeNode);
		bool bFirst = true;

		while(pzStr && cFrom <= strlen(pzStr)) {
			string strK = pzStr;
			string strKeySimple = strK.substr(0, cFrom);
			int iLoop;
			const char * pMatchMix = pTree->GetSSValue_Simple(clsTreeNode, strKeySimple);
			if(!pMatchMix) {
				break;
			}
			vector<string> vecMatch;
			if(bFirst) {
				snprintf(szOut + iMove, sizeof(szOut) - iMove, "{\"len_%d\":\"", cFrom);
				bFirst = 0;
			} else {
				snprintf(szOut + iMove, sizeof(szOut) - iMove, ",{\"len_%d\":\"", cFrom);
			}
			strResult += szOut;
			CCompressPinyin::GetInstance().GetMatchStrs(strKeySimple, szCompress, pMatchMix, strTail, vecMatch);
			for(iLoop = 0; iLoop < vecMatch.size(); iLoop++) {
				strResult += vecMatch[iLoop] + ";";
			}

			snprintf(szOut + iMove, sizeof(szOut) - iMove, "\"}\n");
			strResult += szOut;
			cFrom++;
			if(cFrom & 0x80) {
				break;
			}
			if(ui < (1<<31)) break; //只有一个结果
		}
		snprintf(szOut + iMove, sizeof(szOut) - iMove, "],\n");
		strResult += szOut;
	} else if((clsTreeNode->usHua & XH_TREE_HUA_INFO_SSELF) && (iTreeType & ALL_SELECT_SPELL_TREE)) {
		unsigned int ui = clsTreeNode->UncomposeSpellSelf();
		snprintf(szOut + iMove, sizeof(szOut) - iMove, "\"ss_data_%d\":[\n", (ui & 0x80000000) ? 1 : 0);
		strResult += szOut;
		bool bFirst = true;
		if(ui & (1<<31)) { //多个结果
			int iLoop = 0;
			ui &= ~(1<<31);
			const char * pTmp = (char *)pTree->GetPMove(pTree->m_pstSimpleSelf, ui);
			snprintf(szOut + iMove, sizeof(szOut) - iMove, "{\"len_%d\":\"", (unsigned char)pTmp[iLoop]);
			strResult += szOut;
			iLoop++;
			while(pTmp[iLoop] != 0 && pTmp[iLoop] != '~') {
				strResult += pTmp[iLoop];
				iLoop++;
			}
			strResult += "\"}\n";
			while(1) {
				if(pTmp[iLoop] == 0) {
					break;
				}
				if(pTmp[iLoop] == '~') {
					iLoop++; //指向数字
					snprintf(szOut + iMove, sizeof(szOut) - iMove, ",{\"len_%d\":\"", (unsigned char)pTmp[iLoop]);
					strResult += szOut;
					iLoop++; //指向内容
					while(pTmp[iLoop] != 0 && pTmp[iLoop] != '~') {
						strResult += pTmp[iLoop];
						iLoop++;
					}
					strResult += "\"}\n";
				} else {
					printf("error:----------%s:%d\n", __FILE__, __LINE__);
					break;
				}
			}
		} else {
			const char * pData =  (char *)pTree->GetPMove(pTree->m_pstSimpleSelf, ui);
			strResult += pData;
		}
		snprintf(szOut + iMove, sizeof(szOut) - iMove, "],\n");
		strResult += szOut;
	}
	snprintf(szOut + iMove, sizeof(szOut) - iMove, "\"withpeer\":%d,\n", clsTreeNode->usWPeer);
	strResult += szOut;
	if(clsTreeNode->usHua & XH_TREE_HUA_INFO_WIFE) {
		snprintf(szOut + iMove, sizeof(szOut) - iMove, "\"date\":\"%s\",\n", pTree->GetString(clsTreeNode));
		strResult += szOut;
	} else {
		snprintf(szOut + iMove, sizeof(szOut) - iMove, "\"data\":\"NULL:%s\",\n", pTree->GetString(clsTreeNode));
		strResult += szOut;
	}
	snprintf(szOut + iMove, sizeof(szOut) - iMove, "\"left_node\":[\n");
	strResult += szOut;
	if(clsTreeNode->usHua & XH_TREE_HUA_INFO_LEFT) strResult += Debug_PrintTree(pTree, uiMaxDepth, iTreeType, pTree->GetTreeLeft(clsTreeNode), iDepth+1, string("left"));
	snprintf(szOut + iMove, sizeof(szOut) - iMove, "],\"middle_node\":[\n");
	strResult += szOut;
	if(clsTreeNode->usHua & XH_TREE_HUA_INFO_MID) strResult += Debug_PrintTree(pTree, uiMaxDepth, iTreeType, pTree->GetTreeMid(clsTreeNode), iDepth+1, "middle");
	snprintf(szOut + iMove, sizeof(szOut) - iMove, "],\"right_node\":[\n");
	strResult += szOut;
	if(clsTreeNode->usHua & XH_TREE_HUA_INFO_RIGHT) strResult += Debug_PrintTree(pTree, uiMaxDepth, iTreeType, pTree->GetTreeRight(clsTreeNode), iDepth+1, "right");
	snprintf(szOut + iMove, sizeof(szOut) - iMove, "]\n");
	strResult += szOut;
	strResult += "}";
	return strResult;
}
