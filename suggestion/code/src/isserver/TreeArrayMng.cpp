#include <time.h>
#include <sys/time.h>
#include "isserver/TreeArrayMng.h"
#include "isserver/Memcmp.h"
#include "isserver/TreeArrayStruct.h"
#include "isserver/PacketResult.h"
#include "util/CompressPinyin.h"
#define NOT_TRUE_TOTAL_LEN 10000


#define FNWC_TYPE_ALL 0 // 全部匹配
#define FNWC_TYPE_UP  1 // 把当前转换成up去匹配
#define FNWC_TYPE_LOW 2 // 把当前转换成low去匹配

CTreeArrayMng::CTreeArrayMng()
{
	Init();
}

CTreeArrayMng & CTreeArrayMng::GetInstance()
{
	static CTreeArrayMng cls;
	return cls;
}

void CTreeArrayMng::Init()
{
	m_bNoDotSearch = true; //待增加的属性处理
	m_stSysFlag.bWithMixSearch = true;
	m_stSysFlag.bWithChineseCorrect = true;
	m_stSysFlag.bWithManyTaskSearch = true;
	m_stSysFlag.bWithChineseSpellCorrect = true;
	m_stSysFlag.bWithSpellSimpleCorrect = true;

    m_strPeerAddTaskName = G_STR(addtasksecondname);
}

//查找符合条件的匹配节点
bool CTreeArrayMng::FindMatchNode(vector<CTreeArray*> &vecTreeArray, XHExePara* pxhEPara, VEC_MatchedNode & vecMatchedNode, TopResult & topResult)
{
	char cMatchCategory = pxhEPara->cMatchCategory;         
	int iTreeType = pxhEPara->cTreeType;                    
	int    iPreNullSize = pxhEPara->pParaOrig->iPreNullSize;
	string strPre = pxhEPara->pParaOrig->strPre;            
	string strKey = pxhEPara->strKey;                       
	bool bSameLen = pxhEPara->pParaOrig->bSameLen;
	//CTreeBase * pTree = m_pcXHTree[iTreeType];            
	//if(!pTree || !pTree->m_pstTreeRoot) return false;     
	if(pxhEPara->iTaskSelect == PATTERN_TASK_SELECT_ALL) {
		string strPreT = strPre;
		strPre = "";
		for(int iLoop = 0; iLoop < strPreT.size(); iLoop++) {
			strPre += "0";
		}
	}

	bool bRet = true;                                       
	int iLimit = pxhEPara->pParaOrig->iResNumLimit;
	switch(cMatchCategory) {                                
		case MATCH_CATEGORY_COMMON:       //普通检索        
			bRet = FindMatchNode_Common(vecTreeArray, iTreeType, strPre, strKey, iPreNullSize, vecMatchedNode, bSameLen);
			break;
		case MATCH_CATEGORY_MIXEDSS:     //简拼全拼混合查询
			bRet = FindMatchNode_MixedSS(vecTreeArray, iTreeType, strPre, strKey, iPreNullSize, vecMatchedNode, topResult, bSameLen);
			break;
		case MATCH_CATEGORY_MIXEDCS:     //汉字全拼混合
			bRet = FindMatchNode_MixedCS(vecTreeArray, iTreeType, strPre, strKey, iPreNullSize, vecMatchedNode, topResult, bSameLen);
			break;
		case MATCH_CATEGORY_NINEGRIDKEY: //九宫格匹配
			bRet = FindMatchNode_NineGrid(vecTreeArray, iTreeType, strPre, strKey, iPreNullSize, vecMatchedNode, bSameLen);
			break;
		default:                                            
			bRet = false;                                   
			break;                                          
	}                                                       
	return bRet;                                            
}

bool CTreeArrayMng::CollectResult(const int iTreeType, VEC_MatchedNode& vecMatchedNodes, const int iTotalLen, TopResult & topResult, const bool bSameLen, int iKeyLen)
{
    bool bRet = true;
    int iLoop;
    StForCollect stTmp;
    priority_queue<StForCollect> pqRes;
    int iPQNums = 0;
	if(!bSameLen) {
		iKeyLen = 0;
	}
	PRINT_T("begin collectResult");
	//printf("match count:%d\n", vecMatchedNodes.size());
    for(iLoop = 0; iLoop < vecMatchedNodes.size(); iLoop++) {
	PRINT_T("mid collectResult->a");
        const XHRbNode* pNode  = vecMatchedNodes[iLoop].first;
        CTreeArray *pTreeArray = vecMatchedNodes[iLoop].second;
        CTreeBase * pTree = (CTreeBase*)pTreeArray->GetTree(iTreeType);
	PRINT_T("mid collectResult->a1");
        stTmp.iWeight = pNode->usWeight;
        stTmp.pNode   = pNode;
        stTmp.pTreeArray = (void*)pTreeArray;
	PRINT_T("mid collectResult->a2");
        //printf("----------------------------------------------------------posdif:%d; char:%c, totallen:%d, string:%s\n", stTmp.pNode->cPosDif, stTmp.pNode->cByteDif, iTotalLen, pTree->GetString(stTmp.pNode));
	PRINT_T("mid collectResult->a3");
        if(stTmp.pNode->cPosDif + 1 > iTotalLen) { // || iTotalLen == NOT_TRUE_TOTAL_LEN) {
			if(bSameLen) {
				//即使从中间匹配上了，那么也有可能从左节点中找到最后一个匹配为\0的结果
				if(stTmp.pNode->cPosDif > iTotalLen) {
					continue;
				}
				//查找是否有\0的节点，如果有则只添加这个节点
				const XHRbNode* pNodeLeft = stTmp.pNode; //由于有汉字，可能存在负数，所有也要看右节点
				const XHRbNode* pNodeLeftNew = NULL;
				do { //因为是要求完全相同长度的,因此这里要找到cByteDif为0的节点
					if(pNodeLeft->cByteDif < 0) {
						pNodeLeftNew = pTree->GetTreeRight(pNodeLeft);
					} else if(pNodeLeft->cByteDif  == 0) {
						break;
					} else {
						pNodeLeftNew = pTree->GetTreeLeft(pNodeLeft);
					}
					if(!pNodeLeftNew) break;
					pNodeLeft = pNodeLeftNew;
				} while(1);
				if(pNodeLeft->cByteDif == 0) {
					CollectNode(pTreeArray, iTreeType, pNodeLeft, topResult, iKeyLen);
				}
				continue;
			}
            pqRes.push(stTmp); //从中间就匹配上了
            continue;
        }
	PRINT_T("mid collectResult->b");
        
        const XHRbNode    * pMid   = pTree->GetTreeMid(stTmp.pNode);
        if(!pMid) {
            const void * pWife  = pTree->GetTreeWife(stTmp.pNode);
            if(pWife) {
				const char *pTmp = pTree->GetString(stTmp.pNode);
				//printf("------------------>>>%s:keyLen:%d(but when samelenght set, maybe not select)\n", pTree->GetString(stTmp.pNode), iKeyLen);
				CollectNode(pTreeArray, iTreeType, stTmp.pNode, topResult, iKeyLen);
            } else {
                printf("error:::::==================[%s %d]\n", __FILE__, __LINE__);
            }
        } else {
            stTmp.iWeight = pMid->usWeight;
            stTmp.pNode   = pMid;
            stTmp.pTreeArray = (void*)pTreeArray;
            pqRes.push(stTmp);
            iPQNums++;
        }
	PRINT_T("mid collectResult->c");
    }
	PRINT_T("mid collectResult");
    do {
        if(pqRes.empty()) {
            break;
        }
        StForCollect stInfo = pqRes.top();
        pqRes.pop();
        const XHRbNode *p = stInfo.pNode;
        CTreeArray *pTreeArray = (CTreeArray*)stInfo.pTreeArray;
        CTreeBase * pTree = (CTreeBase*)pTreeArray->GetTree(iTreeType);
        if(p) {
            stTmp.pTreeArray = (void*)pTreeArray;
            const XHRbNode * pLeft  = pTree->GetTreeLeft(p);
            const XHRbNode * pRight = pTree->GetTreeRight(p);
            const XHRbNode * pMid   = pTree->GetTreeMid(p);
            const void     * pWife  = pTree->GetTreeWife(p);
            ADD_NEW_FRANCH(pLeft);
            ADD_NEW_FRANCH(pRight);
			if(!bSameLen) {
				ADD_NEW_FRANCH(pMid);
			}
            if(pWife) {
                CollectNode(pTreeArray, iTreeType, p, topResult, iKeyLen);
            }
        }
    } while(1);
	PRINT_T("after collectResult");
    return bRet;
}

//mapAllTaskInfo all task's extra -> 正常的数据部分
bool CTreeArrayMng::CollectNode(CTreeArray *pTreeArray, const int iTreeType, const XHRbNode* pNode, TopResult & topResult, int iKeyLen)
{
    int iLoop;
    char szLog[32];
    CTreeChinese *pChinese = (CTreeChinese *)pTreeArray->GetTree(SELECT_TREE_CHINESE);
	FlagSet *pFlagSet = topResult.pFlagSet;
	bool bWithFlag = pFlagSet->bWithFlag;
    switch(iTreeType) {
        case SELECT_TREE_CHINESE:
            {
				const char * pStr = pChinese->GetNodeString(pNode);
				if(iKeyLen == 0 || strlen(pStr) == iKeyLen) {
					XHChineseData * pCD = (XHChineseData *)pChinese->GetNodeDataFromId(pChinese->GetIdFromNode(pNode));
					if(!(pCD->ucAttr & FLAG_CHINESE_UCATTR_DELETE) && pFlagSet->IsGoodFlag(pCD->usAttr)) {
						topResult.collect(pChinese->GetIdFromNode(pNode), pCD->usWeight, strlen(pStr), (void*)pChinese);
					}
				}
			}
            break;
		default: //case SELECT_TREE_SPELL: case SELECT_TREE_CENTER_SPELL:
			{
				CTreeSpell * pSpell    = (CTreeSpell   *)pTreeArray->GetTree(iTreeType);
				if(iKeyLen != 0) { //返回完全相同长度结果
					const char *pStringSpell = pSpell->GetString(pNode);
					if(pStringSpell) {
						if(strlen(pStringSpell) != iKeyLen) {
							break;
						}
					}
				}
				const unsigned int cuiUpNum = pSpell->GetNodeExtraNums(pNode);
				//printf("-----------------------------------------------------------------------------------------------collect %u\n", cuiUpNum);
				if(cuiUpNum == 1) { //有1个上级的时候
					unsigned int uiUpId = ((XHSpellData *)(pSpell->GetTreeWife(pNode)))->uiUpperPos;
					XHChineseData * pCD = (XHChineseData *)pChinese->GetNodeDataFromId(uiUpId);
					const char * pStr = (const char *)pChinese->GetNodeStringFromData(pCD);
					//printf("%s---%d\n", pStr, uiUpId);
					if(!pCD) {
						printf("error: pcd is null[%s %d]\n", __FILE__, __LINE__);
						break;
					}
					if(!(pCD->ucAttr & FLAG_CHINESE_UCATTR_DELETE) && pFlagSet->IsGoodFlag(pCD->usAttr)) {
						topResult.collect(uiUpId, pCD->usWeight, strlen(pStr), (void*)pChinese);
					}
				} else {
					const unsigned int * pExtraArray = (const unsigned int *)pSpell->GetNodeExtraInfo(pNode);
					for(iLoop = 0; iLoop < cuiUpNum && iLoop < NUMBER_MAX_RETURN_LIMIT; iLoop++) {
						XHChineseData * pCD = (XHChineseData *)pChinese->GetNodeDataFromId(pExtraArray[iLoop]);
						const char * pStr = (const char *)pChinese->GetNodeStringFromData(pCD);
					//printf("%s---%d\n", pStr, pExtraArray[iLoop]);
						if(!pCD) {
							printf("error: pcd is null[%s %d]\n", __FILE__, __LINE__);
							break;
						}
						if(!(pCD->ucAttr & FLAG_CHINESE_UCATTR_DELETE) && pFlagSet->IsGoodFlag(pCD->usAttr)) {
							topResult.collect(pExtraArray[iLoop], pCD->usWeight, strlen(pStr), (void*)pChinese);
						}
					}
				}
			}
			break;
        //default:
        //    snprintf(szLog, sizeof(szLog), "error:do'nt have this tree(%d)", iTreeType);
        //    printf("%s\n", szLog);
        //    break;
    }
    return true;
}
//strKey->前面部分为处理好的拼音内容，后面部分为包含的汉字实体部分
bool CTreeArrayMng::FindMatchNode_MixedCS(vector<CTreeArray*> &vecTreeArray, const int iTreeType, const string &strPre, const string &strKey, const int iPreNullSize, VEC_MatchedNode &vecMatchedNode, TopResult & topResult, const bool bSameLen)
{
	VEC_MatchedNode vecFull;
	string strKeySpell = "", strKeyPart_Chinese = "";
	int iLoop;
	int iKeyLen = strKey.size();
	if(!bSameLen) { //非“只显示相同长度的结果”
		iKeyLen = 0;
	}
	for(iLoop = 0; iLoop < strKey.size(); iLoop++) {
		if(strKey[iLoop] == '|') {
			break;
		}
		strKeySpell += strKey[iLoop];
	}
	if(iLoop == strKey.size()) {
		return false;
	}
	strKeyPart_Chinese = strKey.substr(iLoop + 1);
	if(!FindMatchNode_Common(vecTreeArray, iTreeType, strPre, strKeySpell, iPreNullSize, vecFull, bSameLen) || vecFull.size() == 0) {
		return false;
	}
//typedef vector<pair<const XHRbNode*, CTreeArray*> > VEC_MatchedNode;
	int jLoop;
	map<const string, vector<CTreeArray*> > mapGoodChinese;
	map<const string, vector<CTreeArray*> >::iterator iIter;
	for(jLoop = 0; jLoop < vecFull.size(); jLoop++) {
		CTreeArray * pTreeArray = vecFull[jLoop].second;
		const XHRbNode   * pxhNode    = vecFull[jLoop].first;
        CTreeBase * pTree = pTreeArray->GetTree(iTreeType);
		const char * pMatchMix = pTree->GetSSValue_Spell(pxhNode, strKeySpell);
		if(!pMatchMix) {
			pxhNode = pTree->GetTreeMid(pxhNode);
			if(pxhNode) {
				pMatchMix = pTree->GetSSValue_Spell(pxhNode, strKeySpell);
			}
			if(!pMatchMix) {
				continue;
			}
		}
		//printf("match----->>%s\n", pMatchMix);
		vector<string> vecMatch;
		string strMatch = "";
		CCompressPinyin::GetInstance().GetMatchStrs(pMatchMix, vecMatch);
		for(iLoop = 0; iLoop < vecMatch.size(); iLoop++) {
			iIter = mapGoodChinese.find(vecMatch[iLoop]);
			if(iIter == mapGoodChinese.end()) {
				mapGoodChinese[vecMatch[iLoop]].push_back(pTreeArray);
				strMatch += "-->" + vecMatch[iLoop];
			} else {
				if(find(iIter->second.begin(), iIter->second.end(), pTreeArray) == iIter->second.end()) {
					iIter->second.push_back(pTreeArray);
					strMatch += "-->" + vecMatch[iLoop];
				}
			}
		}
		//printf("total match string:%s\n", strMatch.c_str());
	}
	int iTreeTypeNew = GetSpellTreeFromSimple(iTreeType);
	int iLimitTime = MAX_MIX_SEARCH_TIME;
	//printf("cs--->>match--------------->>>tree:%d-->>new tree:%d\n", iTreeType, iTreeTypeNew);
	for(iIter = mapGoodChinese.begin(); iIter != mapGoodChinese.end() && iLimitTime; iIter++, iLimitTime--) {
		string strKeyNewFull = iIter->first;
		for(iLoop = 0; iLoop < strKeyPart_Chinese.size(); iLoop += 2) {
			string strChinese = strKeyPart_Chinese.substr(iLoop, 2);
			if(strKeyNewFull.find(strChinese) == string::npos) {
				break;
			}
		}
		if(iLoop < strKeyPart_Chinese.size()) {
			continue;
		}
		int iTotalLen = strPre.size() + strKeyNewFull.size() + iPreNullSize;
		VEC_MatchedNode vecMatchedNodeTmp;
		//if(strKeyNewFull != "生鲜" ) continue;
		FindMatchNode_Common(iIter->second, iTreeTypeNew, strPre, strKeyNewFull, iPreNullSize, vecMatchedNodeTmp, bSameLen);
		//printf("cs--->>match--------------->>>str:%s; %d\n", strKeyNewFull.c_str(), vecMatchedNodeTmp.size());
		if(vecMatchedNodeTmp.size() > 0) {
			CollectResult(iTreeTypeNew, vecMatchedNodeTmp, iTotalLen, topResult, bSameLen, iKeyLen);
		}
	}

	return true;
}
bool CTreeArrayMng::FindMatchNode_MixedSS(vector<CTreeArray*> &vecTreeArray, const int iTreeType, const string &strPre, const string &strKey, const int iPreNullSize, VEC_MatchedNode &vecMatchedNode, TopResult & topResult, const bool bSameLen)
{
	VEC_MatchedNode vecSimple;
	string strKeySimple = "";
	char szCompress[1024];
	int iLoop, iSize = strKey.size();
	string strTail = "";
	string strNewKey = "";
	int iKeyLen = strKey.size();
	if(bSameLen) { //非“只显示相同长度的结果”
		iKeyLen = 0;
	}
	for(iLoop = 0; iLoop < iSize; iLoop++) {
		char c = strKey[iLoop];
		if(c >= 'A' && c <= 'Z') {
			strKeySimple += (c - 'A' + 'a');
		} else if(c >= 'a' && c <= 'z') {
		} else {
			strKeySimple += c;
		}
	}
	for(iLoop = iSize - 1; iLoop > 0; iLoop--) {
		char c = strKey[iLoop];
		if(c >= 'A' && c <= 'Z') {
			break;
		}
	}
	if(iLoop == iSize - 1) {
		strTail = "";
		strNewKey = strKey;
	} else {
		strTail = strKey.substr(iLoop + 1);
		strNewKey = strKey.substr(0, iLoop + 1);
	}
	int iSimpleSize = strKeySimple.size();
	if(!CCompressPinyin::GetInstance().CompressStr(iSimpleSize, strNewKey, szCompress)) {
		printf("error:CCompressPinyin::GetInstance().CompressStr:%d,%s[%s %d]\n", iSimpleSize, strNewKey.size(), __FILE__, __LINE__);
		return false;
	}
	if(!FindMatchNode_Common(vecTreeArray, iTreeType, strPre, strKeySimple, iPreNullSize, vecSimple, bSameLen) || vecSimple.size() == 0) {
		return false;
	}
//typedef vector<pair<const XHRbNode*, CTreeArray*> > VEC_MatchedNode;
	int jLoop;
	map<const string, vector<CTreeArray*> > mapGoodPinyin;
	map<const string, vector<CTreeArray*> >::iterator iIter;
	for(jLoop = 0; jLoop < vecSimple.size(); jLoop++) {
		CTreeArray * pTreeArray = vecSimple[jLoop].second;
		const XHRbNode   * pxhNode    = vecSimple[jLoop].first;
        CTreeBase * pTree = pTreeArray->GetTree(iTreeType);
		const char * pMatchMix = pTree->GetSSValue_Simple(pxhNode, strKeySimple);
		if(!pMatchMix) {
			continue;
		}
		vector<string> vecMatch;
		CCompressPinyin::GetInstance().GetMatchStrs(strKeySimple, szCompress, pMatchMix, strTail, vecMatch);
		for(iLoop = 0; iLoop < vecMatch.size(); iLoop++) {
			iIter = mapGoodPinyin.find(vecMatch[iLoop]);
			if(iIter == mapGoodPinyin.end()) {
				mapGoodPinyin[vecMatch[iLoop]].push_back(pTreeArray);
			} else {
				if(find(iIter->second.begin(), iIter->second.end(), pTreeArray) == iIter->second.end()) {
					iIter->second.push_back(pTreeArray);
				}
			}
		}
	}
	int iTreeTypeNew = GetSpellTreeFromSimple(iTreeType);
	int iLimitTime = MAX_MIX_SEARCH_TIME;
	//printf("ss--->>match--------------->>>tree:%d-->>new tree:%d\n", iTreeType, iTreeTypeNew);
	for(iIter = mapGoodPinyin.begin(); iIter != mapGoodPinyin.end() && iLimitTime; iIter++, iLimitTime--) {
		string strKeyNewFull = iIter->first;
		int iTotalLen = strPre.size() + strKeyNewFull.size() + iPreNullSize;
		VEC_MatchedNode vecMatchedNodeTmp;
		printf("ss--->>match--------------->>>str:%s\n", strKeyNewFull.c_str());
		FindMatchNode_Common(iIter->second, iTreeTypeNew, strPre, strKeyNewFull, iPreNullSize, vecMatchedNodeTmp, bSameLen);
		if(vecMatchedNodeTmp.size() > 0) {
			CollectResult(iTreeTypeNew, vecMatchedNodeTmp, iTotalLen, topResult, bSameLen, iKeyLen);
		}
	}
	return true;
}
int CTreeArrayMng::GetSpellTreeFromSimple(const int iTree)
{
	int iSpellTree;
	if(iTree == SELECT_TREE_SPELL) return SELECT_TREE_CHINESE;
	if(iTree == SELECT_TREE_CENTER_SPELL) return SELECT_TREE_CENTER_CHINESE;
	if(iTree == SELECT_TREE_EX_SPELL) return SELECT_TREE_EX_CHINESE;
	if(iTree == SELECT_TREE_EX_CENTER_SPELL) return SELECT_TREE_EX_CENTER_CHINESE;
	if(iTree == SELECT_TREE_SIMPLE) return SELECT_TREE_SPELL;
	if(iTree == SELECT_TREE_CENTER_SIMPLE) return SELECT_TREE_CENTER_SPELL;
	if(iTree == SELECT_TREE_EX_SIMPLE) return SELECT_TREE_EX_SPELL;
	//if(iTree == SELECT_TREE_EX_CENTER_SIMPLE)
	return SELECT_TREE_EX_CENTER_SPELL;
}
//九宫格
//现在在收集左右节点的时候速度还有些慢,有待提高
bool CTreeArrayMng::FindMatchNode_NineGrid(vector<CTreeArray*> &vecTreeArray, const int iTreeType, const string &strPre, const string &strKey, const int iPreNullSize, VEC_MatchedNode &vecMatchedNode, const bool bSameLen)
{
    queue<pair<const XHRbNode*,CTreeArray*> > queueNode;
    int iLoop, iSizeTmp = vecTreeArray.size();
    for(iLoop = 0; iLoop < iSizeTmp; iLoop++) {
        CTreeBase * pTree = vecTreeArray[iLoop]->GetTree(iTreeType);
        const XHRbNode * pRoot = pTree->GetTreeRoot();
        if(!pRoot) return false;
        queueNode.push(make_pair(pRoot, vecTreeArray[iLoop]));
    }

    int iPreLen = strPre.size();
    int iTotalLen = iPreLen + iPreNullSize + strKey.size();
	int iPreAllLen = iPreLen + iPreNullSize;
	bool bSpellMatch = 0;
	if(iTreeType == SELECT_TREE_SPELL || iTreeType == SELECT_TREE_CENTER_SPELL
		|| iTreeType == SELECT_TREE_EX_SPELL || iTreeType == SELECT_TREE_EX_CENTER_SPELL) {
		bSpellMatch = 1;
	}
	//                    a b c d e f g h i j k l m n o p q r s t u v w x y z
	static int szLow[] = {2,2,2,3,3,3,4,4,4,5,5,5,6,6,6,7,7,7,7,8,8,8,9,9,9,9};
	//一个数字对应的最大的字母
	//数组大小为2,分别为bSpellMatch的0,1-->>因为小写的ascii码大于大写的,因此上限是固定的
	//                             从0开始,2才有效
	//                                0 1  2   3   4   5   6   7   8   9
	static char szNumberUp[]      =  {0,0,'c','f','i','l','o','s','v','z'};
	static char szNumberLow[][10] = {{0,0,'a','d','g','j','m','p','t','w'},
	                                 {0,0,'A','D','G','J','M','P','T','W'}};
	static char sCharToNum[0xff]  = {0};
	static bool bFirst = true;
	if(bFirst) {
		bFirst = false;
		for(iLoop = 0; iLoop < sizeof(szLow) / sizeof(szLow[0]); iLoop++) {
			sCharToNum['a' + iLoop] = szLow[iLoop] + '0';
			sCharToNum['A' + iLoop] = szLow[iLoop] + '0';
		}
	}
	PRINT_T("before find");
	int iCount = 0;
	int iOverSize = iPreLen + iPreNullSize + strKey.size();
    while(!queueNode.empty()) {
		iCount++;
        const XHRbNode *pNode        = (XHRbNode*)queueNode.front().first;
        CTreeArray *pTreeArray = (CTreeArray*)queueNode.front().second;
        CTreeBase *pTree       = (CTreeBase*)pTreeArray->GetTree(iTreeType);
        queueNode.pop();
        if(pNode->cPosDif >= iTotalLen) {
			if(pNode->cPosDif > iTotalLen && bSameLen) {
				continue;
			}
            const char *szT = pTree->GetString(pNode);
			if(MEMCMP_NINEGRID(strKey.c_str(), szT, strKey.size()) == 0) {
				vecMatchedNode.push_back(make_pair(pNode, pTreeArray));
			}
        } else if(pNode->cPosDif < iPreLen) {
            char c = strPre[pNode->cPosDif];
            char cNode = pNode->cByteDif;
            if(c < cNode) {
                const XHRbNode * pLeft  = pTree->GetTreeLeft(pNode);
                if(pLeft)  queueNode.push(make_pair(pLeft, pTreeArray));
            } else if(c == cNode) {
                const XHRbNode * pMid   = pTree->GetTreeMid(pNode);
                if(pMid)   queueNode.push(make_pair(pMid, pTreeArray));
            } else {
                const XHRbNode * pRight = pTree->GetTreeRight(pNode);
                if(pRight) queueNode.push(make_pair(pRight, pTreeArray));
            }
        } else if(pNode->cPosDif >= iPreLen && pNode->cPosDif < iPreLen + iPreNullSize) {
            const XHRbNode * pLeft  = pTree->GetTreeLeft(pNode);
            const XHRbNode * pRight = pTree->GetTreeRight(pNode);
            const XHRbNode * pMid   = pTree->GetTreeMid(pNode);
            if(pLeft)  queueNode.push(make_pair(pLeft, pTreeArray));
            if(pRight) queueNode.push(make_pair(pRight, pTreeArray));
            if(pMid)   queueNode.push(make_pair(pMid, pTreeArray));
        } else {
            const XHRbNode * pLeft  = pTree->GetTreeLeft(pNode);
            const XHRbNode * pRight = pTree->GetTreeRight(pNode);
            const XHRbNode * pMid   = pTree->GetTreeMid(pNode);
			char cNode = pNode->cByteDif;
			char cNodeNum = sCharToNum[cNode];
            char c = strKey[pNode->cPosDif - iPreLen - iPreNullSize];

            if(pLeft && cNode > szNumberLow[bSpellMatch][c - '0']) {
				queueNode.push(make_pair(pLeft, pTreeArray));
			}
            if(pRight && cNode < szNumberUp[c - '0']) {
				queueNode.push(make_pair(pRight, pTreeArray));
			}
			if(cNodeNum == c) { //两个字母相等(转换成数字之后)
				if(pMid && pNode->cPosDif < iOverSize) {
					queueNode.push(make_pair(pMid, pTreeArray));
				} else { //没有中孩子
					const char *szT = pTree->GetString(pNode);
					int iLenTmp = strlen(szT);
					if(((iLenTmp > strKey.size() && !bSameLen) || iLenTmp ==strKey.size()) && MEMCMP_NINEGRID(strKey.c_str(), szT, strKey.size()) == 0) {
						vecMatchedNode.push_back(make_pair(pNode, pTreeArray));
					}
				}
			}
        }
    }
	char szCount[32];
	snprintf(szCount, sizeof(szCount), "loop times:%d", iCount);
	PRINT_T(szCount);
    return true;
}
bool CTreeArrayMng::FindMatchNode_Common(vector<CTreeArray*> &vecTreeArray, const int iTreeType, const string &strPre, const string &strKey, const int iPreNullSize, VEC_MatchedNode &vecMatchedNode, const bool bSameLen)
{
    queue<pair<const XHRbNode*,CTreeArray*> > queueNode;
    int iLoop, iSizeTmp = vecTreeArray.size();
    for(iLoop = 0; iLoop < iSizeTmp; iLoop++) {
        CTreeBase * pTree = vecTreeArray[iLoop]->GetTree(iTreeType);
        const XHRbNode * pRoot = pTree->GetTreeRoot();
        if(!pRoot) return false;
        queueNode.push(make_pair(pRoot, vecTreeArray[iLoop]));
    }

    int iPreLen = strPre.size();
    int iTotalLen = iPreLen + iPreNullSize + strKey.size();
	int iPreAllLen = iPreLen + iPreNullSize;
	bool bSpellMatch = 0;
	if(iTreeType == SELECT_TREE_SPELL || iTreeType == SELECT_TREE_CENTER_SPELL
		|| iTreeType == SELECT_TREE_EX_SPELL || iTreeType == SELECT_TREE_EX_CENTER_SPELL) {
		bSpellMatch = 1;
	}

	PRINT_T("before find");
	int iCount = 0;
	int iOverSize = iPreLen + iPreNullSize + strKey.size() - 1;
    while(!queueNode.empty()) {
		iCount++;
        const XHRbNode *pNode        = (XHRbNode*)queueNode.front().first;
        CTreeArray *pTreeArray = (CTreeArray*)queueNode.front().second;
        CTreeBase *pTree       = (CTreeBase*)pTreeArray->GetTree(iTreeType);
        queueNode.pop();
        if(pNode->cPosDif >= iTotalLen) {
			if(pNode->cPosDif > iTotalLen && bSameLen) {
				continue;
			}
            const char *szT = pTree->GetString(pNode);
			if(!bSpellMatch) {
				if(memcmp(szT, strKey.c_str(), strKey.size()) == 0) {
					vecMatchedNode.push_back(make_pair(pNode, pTreeArray));
				} else {
					continue;
				}
			} else {
				if(MEMCMP(szT, strKey.c_str(), strKey.size()) == 0) {
					vecMatchedNode.push_back(make_pair(pNode, pTreeArray));
				} else {
					continue;
				}
			}
        } else if(pNode->cPosDif < iPreLen) {
            char c = strPre[pNode->cPosDif];
            char cNode = pNode->cByteDif;
            if(c < cNode) {
                const XHRbNode * pLeft  = pTree->GetTreeLeft(pNode);
                if(pLeft)  queueNode.push(make_pair(pLeft, pTreeArray));
            } else if(c == cNode) {
                const XHRbNode * pMid   = pTree->GetTreeMid(pNode);
                if(pMid) {
					queueNode.push(make_pair(pMid, pTreeArray));
				} else {
					const char *szT = pTree->GetString(pNode);
					int iLenTmp = strlen(szT);
					if(((iLenTmp > strKey.size() && !bSameLen) || iLenTmp ==strKey.size()) && (bSpellMatch && MEMCMP(szT, strKey.c_str(), strKey.size()) == 0 || !bSpellMatch && memcmp(szT, strKey.c_str(), strKey.size()) == 0)) {
						vecMatchedNode.push_back(make_pair(pNode, pTreeArray));
					}
				}
            } else {
                const XHRbNode * pRight = pTree->GetTreeRight(pNode);
                if(pRight) queueNode.push(make_pair(pRight, pTreeArray));
            }
        } else if(pNode->cPosDif >= iPreLen && pNode->cPosDif < iPreLen + iPreNullSize) {
            const XHRbNode * pLeft  = pTree->GetTreeLeft(pNode);
            const XHRbNode * pRight = pTree->GetTreeRight(pNode);
            const XHRbNode * pMid   = pTree->GetTreeMid(pNode);
            const void    * pWife  = pTree->GetTreeWife(pNode);
            if(pLeft)  queueNode.push(make_pair(pLeft, pTreeArray));
            if(pRight) queueNode.push(make_pair(pRight, pTreeArray));
            if(pMid)   queueNode.push(make_pair(pMid, pTreeArray));
			if(pWife) {
				const char *szT = pTree->GetString(pNode);
				int iLenTmp = strlen(szT);
				if(((iLenTmp > strKey.size() && !bSameLen) || iLenTmp ==strKey.size()) && (bSpellMatch && MEMCMP(szT, strKey.c_str(), strKey.size()) == 0 || !bSpellMatch && memcmp(szT, strKey.c_str(), strKey.size()) == 0)) {
					vecMatchedNode.push_back(make_pair(pNode, pTreeArray));
				}
			}
        } else {
            char c = strKey[pNode->cPosDif - iPreLen - iPreNullSize];
			if(bSpellMatch && (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')) {
				//先查找大写,然后查找小写,如果没有大写结果或者大写结果提示没有peer结果,则进行小写查找(但如果c为大写,也不会进行小写查找)
                const XHRbNode * pCase[2];
                pCase[1] = NULL; //小写
                pCase[0] = FindNodeWithCase(pTree, strKey, iPreAllLen, pNode, FNWC_TYPE_UP);
                if((!pCase[0] || pCase[0]->usWPeer) && (c >= 'a' && c <= 'z')) {
                    pCase[1] = FindNodeWithCase(pTree, strKey, iPreAllLen, pNode, FNWC_TYPE_LOW);
                    if(pCase[0] == pCase[1]) pCase[1] = NULL;
                }
                for(int iLoop = 0; iLoop < 2; iLoop++) {
                    if(pCase[iLoop]) {
                        const XHRbNode * pMid   = pTree->GetTreeMid(pCase[iLoop]);
                        if(pMid && pCase[iLoop]->cPosDif < iOverSize) {
                            queueNode.push(make_pair(pMid, pTreeArray));
                        } else { //没有中孩子
                            const char *szT = pTree->GetString(pCase[iLoop]);
							int iLenTmp = strlen(szT);
                            if(((iLenTmp > strKey.size() && !bSameLen) || iLenTmp ==strKey.size()) && MEMCMP(szT, strKey.c_str(), strKey.size()) == 0) {
                                vecMatchedNode.push_back(make_pair(pCase[iLoop], pTreeArray));
                            }
                        }
                    }
                }
            } else { //非字母
				const XHRbNode * pMatch = FindNodeWithCase(pTree, strKey, iPreAllLen, pNode, FNWC_TYPE_ALL);
				if(pMatch) {
					const XHRbNode * pMid   = pTree->GetTreeMid(pMatch);
					if(pMid && pMatch->cPosDif < iOverSize) {
						queueNode.push(make_pair(pMid, pTreeArray));
					} else { //没有中孩子
						const char *szT = pTree->GetString(pMatch);
						if(!bSpellMatch) {
							if(strlen(szT) >= strKey.size() && memcmp(szT, strKey.c_str(), strKey.size()) == 0) {
								vecMatchedNode.push_back(make_pair(pMatch, pTreeArray));
							}
						} else { //如果是数字字母混合,并且以数字结尾
							//todo==
							//这里可能有问题,就是带有数字的时候
							int iLenTmp = strlen(szT);
                            if(((iLenTmp > strKey.size() && !bSameLen) || iLenTmp ==strKey.size()) && MEMCMP(szT, strKey.c_str(), strKey.size()) == 0) {
								vecMatchedNode.push_back(make_pair(pMatch, pTreeArray));
							}
						}
					}
				}
            }
        }
    }
	char szCount[32];
	snprintf(szCount, sizeof(szCount), "loop times:%d", iCount);
	PRINT_T(szCount);
    return true;
}

//iType: FNWC_TYPE_ALL  全部匹配
//iType: FNWC_TYPE_UP   把当前转换成up去匹配
//iType: FNWC_TYPE_LOW  把当前转换成low去匹配
//iType: FNWC_TYPE_KEEP 把当前的字母保持不变去匹配
const XHRbNode *CTreeArrayMng::FindNodeWithCase(CTreeBase *pTree, const string & strKey, const int iPreAllSize, const XHRbNode * &pNode, const int iType)
{
    int iTotalLen = iPreAllSize + strKey.size();
    char c = strKey[pNode->cPosDif - iPreAllSize];
    char cCase = c;
	switch(iType) {
		case FNWC_TYPE_UP:
				if(cCase >= 'a' && cCase <= 'z') {
					cCase = cCase - 'a' + 'A';
				}
			break;
		case FNWC_TYPE_LOW:
			if(cCase >= 'A' && cCase <= 'Z') {
				cCase = cCase - 'A' + 'a';
			} 
			break;
		default: //FNWC_TYPE_ALL, FNWC_TYPE_KEEP
			break;
	}
    const XHRbNode * p = pNode;
    while(p) {
        char cNode = p->cByteDif;
        if(cCase < cNode) {
            p  = pTree->GetTreeLeft(p);
        } else if(cCase == cNode) {
            return p;
        } else {
            p  = pTree->GetTreeRight(p);
        }
    }
    return NULL;
}

bool CTreeArrayMng::SearchCategory(vector<CTreeArray*> &vecTreeArray, XHExePara* pxhEPara, TopResult & topResult)
{
	VEC_MatchedNode vecMatched;
	string strKey = pxhEPara->strKey;
	int iTreeType = pxhEPara->cTreeType;
	int iTotalLen = pxhEPara->GetTotalLen();
	bool bSameLen = pxhEPara->pParaOrig->bSameLen;
	if(!FindMatchNode(vecTreeArray, pxhEPara, vecMatched, topResult)) {
		return false;
	}
	if(pxhEPara->cMatchCategory != MATCH_CATEGORY_MIXEDSS && pxhEPara->cMatchCategory != MATCH_CATEGORY_MIXEDCS) {
		if(vecMatched.size() == 0) {
			return false;
		}
		int iKeyLen = strKey.size();
		//所有的ikeylen都是为bsamelen使用
		PRINT_T("before 1collectResult");
		CollectResult(iTreeType, vecMatched, iTotalLen, topResult, bSameLen, iKeyLen);
		PRINT_T("after 2collectResult");
	}

	return true;
}

//再次注释这里
//mapTreeInfo 要使用传值的方法进行传递,因为是多线程的操作,又没有使用锁
//我认为这样能解决没有锁但是多个线程同时访问的问题,我是这样认为的
//可能有更好的方法,暂时先这样吧
bool CTreeArrayMng::SearchResult(map<const string, string> & mapKeyValue, map<const string, CTMInfo *> mapTreeInfo, string & strResult)
{
	CXHPara xhPara(mapKeyValue, &m_stSysFlag);
	PRINT_T("after anly category");
	string strTaskName = xhPara.GetTaskName();
	vector<CTreeArray*> vecAllTask;
	vector<CTreeArray*> vecCurTask;
	//初始化task选择模块
	map<const string, CTMInfo*>::iterator iIterMap;
	string strPre = xhPara.GetPre();
	string strTaskSecondName = m_strPeerAddTaskName + strTaskName;
	for(iIterMap = mapTreeInfo.begin(); iIterMap != mapTreeInfo.end(); iIterMap++) {
		CTreeArray * pTreeArray = iIterMap->second->GetTreeArray();
		if(iIterMap->first == strTaskName) {
			iIterMap->second->m_ulHitTime++;
			vecCurTask.push_back(pTreeArray);
			if(strPre == "") { //前缀为空,不选择当前的task了
				continue;
			}
		}
		if(iIterMap->first == strTaskSecondName) {
			iIterMap->second->m_ulHitTime++;
			vecCurTask.push_back(pTreeArray);
			if(strPre == "") { //前缀为空,不选择当前的task了
				continue;
			}
		}
		vecAllTask.push_back(pTreeArray);
	}
	
	//查询收集结果
	int iLoop;
	int iRetLimit = xhPara.GetLimitNumber();
	int iCategoryNum = xhPara.GetCategoryNum();
	TopResult topResult(iRetLimit);
	topResult.pFlagSet = &(xhPara.stFlagSet);
	topResult.bSimpleView = xhPara.bSimpleView;
	bool bSCRet = true;
	for(iLoop = 0; iLoop < iCategoryNum; iLoop++) {
		PRINT_T("before look category");
		XHExePara * pxhEP = xhPara.GetCategory(iLoop);
		PRINT_T("before print para category");
		pxhEP->Print(); //打印策略-->debug使用
		if(pxhEP->iTaskSelect == PATTERN_TASK_SELECT_CUR && !xhPara.iMustAllTask) { //当前task
			if(strPre == "") { //如果前缀为空,那么直接使用ex查询
				pxhEP->iTaskSelect = PATTERN_TASK_SELECT_ALL;
			}
			if(vecCurTask.size() == 0) {
				continue;
			} else {
				bSCRet = SearchCategory(vecCurTask, pxhEP, topResult);
			}
		} else { //全部task
			bSCRet = SearchCategory(vecAllTask, pxhEP, topResult);
		}
		if(topResult.getSavedSize() >= iRetLimit) { //已经收集够了
			break;
		}
		if(!bSCRet) { //收集有问题,忽略本次收集
			continue;
		}
		PRINT_T("after search category");
		int iAddWeight = pxhEP->iAddWeight;
		if(iAddWeight > 0) {
			topResult.AddWeight(iAddWeight);
		}
		//printf("--------------------------->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>%d\n", iAddWeight);
		PRINT_T("after look category");
	}
	CPacketResult cPR;
	PRINT_T("before packet");
	strResult = cPR.Packet(topResult);
	PRINT_T("after packet");
	//printf("%s\n", strResult.c_str());
	return true;
}
