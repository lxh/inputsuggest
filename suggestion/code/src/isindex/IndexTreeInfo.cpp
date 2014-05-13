#include "isindex/IndexTreeInfo.h"
#include "util/ReadBigFile.h"
#include "util/XHStrUtils.h"
#include "util/WriteLog.h"
#include "util/CompressPinyin.h"

int CIndexTreeInfo::Run(const string & strTaskName, CSysConfigSet *pclsSysConfigSet)
{
	m_strTaskName = strTaskName;
	m_pSysConfigSet = pclsSysConfigSet;
	m_iPrefixLen = -1;
	InitParamter();
	Process();
	return 0;
}

void CIndexTreeInfo::InitParamter()
{
	m_strDataPath = m_pSysConfigSet->GetString(PARA_CONFIG_pathdata);
	//初始化要生成的dict
	string strDictNeed = m_pSysConfigSet->GetString(PARA_CONFIG_selecttree);
	vector<string> vecDictNeed;
	XHStrUtils::StrTokenize(vecDictNeed, strDictNeed, ";");
	int iLoop;
	//executed by id asc
	memset(m_bDictNeedCreate, 0x00, sizeof(m_bDictNeedCreate));
	for(iLoop = 0; iLoop < vecDictNeed.size(); iLoop++) {
		int iSub = atoi(vecDictNeed[iLoop].c_str());
		if(iSub >= 0 && iSub <= sizeof(m_bDictNeedCreate) / sizeof(m_bDictNeedCreate[0])) {
			m_bDictNeedCreate[iSub] = 1;
		}
	}
}

//释放掉在申请树节点时,申请的内存
void CIndexTreeInfo::FreeTreeNodeData(CTreeNode * & clsTreeNode)
{           
	if(clsTreeNode->pleft_)   FreeTreeNodeData(clsTreeNode->pleft_);
	if(clsTreeNode->pmiddle_) FreeTreeNodeData(clsTreeNode->pmiddle_);
	if(clsTreeNode->pright_)  FreeTreeNodeData(clsTreeNode->pright_);
	if(clsTreeNode->pitem_) {
		if(clsTreeNode->pitem_->pkey_) {
			delete []clsTreeNode->pitem_->pkey_;
			clsTreeNode->pitem_->pkey_ = NULL;
		}
		delete clsTreeNode->pitem_;
		clsTreeNode->pitem_ = NULL;
	}   
}
//遍历一个节点的所有的兄弟节点(left,right)
bool CIndexTreeInfo::CollectAllBrother(CTreeNode * & pTreeNode, map<const int, map<const char, vector<CTreeNode*> > >& mapT)
{
    if(pTreeNode->pleft_) {
        CollectAllBrother(pTreeNode->pleft_, mapT);
    }
    if(pTreeNode->pright_) {
        CollectAllBrother(pTreeNode->pright_, mapT);
    }
    char c = pTreeNode->cbyte_;
    if(c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {
        mapT[pTreeNode->sbyte_index_][c].push_back(pTreeNode);
        //printf("%c(%d) ", c, pTreeNode->sbyte_index_);
    }
    return true;
}
//如果有中孩子,把所有中孩子的兄弟们（中孩子的左右孩子的递归）都获得,然后判断在不同字节位置相同的地方的值是否存在多个
bool CIndexTreeInfo::MarkHavePeerInfo(CTreeNode * & pTreeNode)
{
    bool bRet = true;
    pTreeNode->bHavePeer = false;

    if(pTreeNode->pleft_)   MarkHavePeerInfo(pTreeNode->pleft_);
    if(pTreeNode->pright_)  MarkHavePeerInfo(pTreeNode->pright_);
    if(pTreeNode->pmiddle_) {
        MarkHavePeerInfo(pTreeNode->pmiddle_);
        map<const int, map<const char, vector<CTreeNode*> > > mapF;
        map<const int, map<const char, vector<CTreeNode*> > >::iterator iIterAll;
        map<const char, vector<CTreeNode*> >::iterator iIterPart, iIterPart2;
        //printf("begin:");
        CollectAllBrother(pTreeNode->pmiddle_, mapF);
        //printf("\n");
        for(iIterAll = mapF.begin(); iIterAll != mapF.end(); iIterAll++) {
            for(iIterPart = iIterAll->second.begin(); iIterPart != iIterAll->second.end(); iIterPart++) {
                char c = iIterPart->first;
                if(c >= 'a' && c <= 'z') {
                    char cPeer = c - 'a' + 'A';
                    iIterPart2 = iIterAll->second.find(cPeer);
                    if(iIterPart2 != iIterAll->second.end()) { //找到
                        for(int iLoop = 0; iLoop < iIterPart->second.size(); iLoop++) {
                            iIterPart->second[iLoop]->bHavePeer = true;
                        }
                        for(int iLoop = 0; iLoop < iIterPart2->second.size(); iLoop++) {
                            iIterPart2->second[iLoop]->bHavePeer = true;
                        }
                        //printf("--------->>>peer:%c\n", c);
                    }
                }
            }
        }
    }
    return bRet;
}

bool CIndexTreeInfo::GetCompressMapping(const int iSelectId, IdToId * & pstIdToId)
{
	int iSelectIdNew = -1;
	switch(iSelectId) {
		case SELECT_TREE_EX_CHINESE        : iSelectIdNew = SELECT_TREE_CHINESE       ; break;
		case SELECT_TREE_EX_SPELL          : iSelectIdNew = SELECT_TREE_SPELL         ; break;
		case SELECT_TREE_EX_SIMPLE         : iSelectIdNew = SELECT_TREE_SIMPLE        ; break;
		case SELECT_TREE_EX_CENTER_CHINESE : iSelectIdNew = SELECT_TREE_CENTER_CHINESE; break;
		case SELECT_TREE_EX_CENTER_SPELL   : iSelectIdNew = SELECT_TREE_CENTER_SPELL  ; break;
		case SELECT_TREE_EX_CENTER_SIMPLE  : iSelectIdNew = SELECT_TREE_CENTER_SIMPLE ; break;
	}
	if(iSelectIdNew == -1) return false;
	if(!m_bDictNeedCreate[iSelectIdNew]) {
		return false;
	}
	string strPath = m_strDataPath + "/" + m_strTaskName + "/";
	string strFileName = strPath + m_pSysConfigSet->GetFileName(iSelectId);
	string strNodeData = strFileName + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_nodedata);
	int iFd = open(strNodeData.c_str(), O_RDONLY);
	if(iFd == -1) {
		char szInfo[1024];
		snprintf(szInfo, sizeof(szInfo), "error: file(%s) open failed\n", strNodeData.c_str());
		CWriteLog::GetInstance().WriteLog(szInfo);
		return false;
	}
	unsigned int uiSize;
	if(read(iFd, &uiSize, 4) != 4) {
		char szInfo[1024];
		snprintf(szInfo, sizeof(szInfo), "error: file(%s) read 4 byte failed\n", strNodeData.c_str());
		CWriteLog::GetInstance().WriteLog(szInfo);
		close(iFd);
		return false;
	}
	uiSize = (uiSize - 4) / sizeof(IdToId);
	pstIdToId = new IdToId[uiSize];
	if(pstIdToId == NULL) {
		char szInfo[1024];
		snprintf(szInfo, sizeof(szInfo), "error: file(%s) new %d IdToId failed\n", strNodeData.c_str(), uiSize);
		CWriteLog::GetInstance().WriteLog(szInfo);
		close(iFd);
		return false;
	}
	
	if(read(iFd, (char *)pstIdToId, uiSize * sizeof(IdToId)) != uiSize * sizeof(IdToId)) {
		char szInfo[1024];
		snprintf(szInfo, sizeof(szInfo), "error: file(%s) new %d IdToId failed2\n", strNodeData.c_str(), uiSize);
		CWriteLog::GetInstance().WriteLog(szInfo);
		close(iFd);
		SAFE_DELETE_MUL(pstIdToId);
		return false;
	}
	close(iFd);
	return true;
}
//读取汉字的nodedata,chinesestring
bool CIndexTreeInfo::GetChineseDataAndString(const int iSelectId, XHChineseData * & pXHCD, char * &pChineseString)
{
	if(iSelectId != SELECT_TREE_SPELL && iSelectId != SELECT_TREE_EX_SPELL) {
		return false;
	}
	char szInfo[1024];
	string strPath = m_strDataPath + "/" + m_strTaskName + "/";
	string strFileName = strPath + m_pSysConfigSet->GetFileName(SELECT_TREE_CHINESE);
	string strNodeData = strFileName + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_nodedata);
	string strString   = strPath + m_pSysConfigSet->GetString(PARA_CONFIG_chinesestring);
	{
		int iFd = open(strNodeData.c_str(), O_RDONLY);
		if(iFd == -1) {
			snprintf(szInfo, sizeof(szInfo), "error: file(%s) open failed\n", strNodeData.c_str());
			CWriteLog::GetInstance().WriteLog(szInfo);
			return false;
		}
		unsigned int uiSize;
		if(read(iFd, &uiSize, 4) != 4) {
			snprintf(szInfo, sizeof(szInfo), "error: file(%s) read 4 byte failed\n", strNodeData.c_str());
			CWriteLog::GetInstance().WriteLog(szInfo);
			close(iFd);
			return false;
		}
		uiSize = (uiSize - 4) / sizeof(XHChineseData);
		pXHCD = new XHChineseData[uiSize];
		if(pXHCD == NULL) {
			snprintf(szInfo, sizeof(szInfo), "error: file(%s) new %d XHChineseData failed\n", strNodeData.c_str(), uiSize);
			CWriteLog::GetInstance().WriteLog(szInfo);
			close(iFd);
			return false;
		}

		if(read(iFd, (char *)pXHCD, uiSize * sizeof(XHChineseData)) != uiSize * sizeof(XHChineseData)) {
			snprintf(szInfo, sizeof(szInfo), "error: file(%s) new %d XHChineseData failed2\n", strNodeData.c_str(), uiSize);
			CWriteLog::GetInstance().WriteLog(szInfo);
			close(iFd);
			SAFE_DELETE_MUL(pXHCD);
			return false;
		}
		close(iFd);
	}
	{
		int iFd = open(strString.c_str(), O_RDONLY);
		if(iFd == -1) {
			snprintf(szInfo, sizeof(szInfo), "error: file(%s) open failed\n", strString.c_str());
			CWriteLog::GetInstance().WriteLog(szInfo);
			return false;
		}
		unsigned int uiSize;
		if(read(iFd, &uiSize, 4) != 4) {
			snprintf(szInfo, sizeof(szInfo), "error: file(%s) read 4 byte failed\n", strString.c_str());
			CWriteLog::GetInstance().WriteLog(szInfo);
			close(iFd);
			return false;
		}
		pChineseString = new char[uiSize];
		if(pChineseString == NULL) {
			snprintf(szInfo, sizeof(szInfo), "error: file(%s) new %d char failed\n", strString.c_str(), uiSize);
			CWriteLog::GetInstance().WriteLog(szInfo);
			close(iFd);
			return false;
		}
		((unsigned int*)pChineseString)[0] = uiSize;

		if(read(iFd, (char *)(pChineseString + 4), (uiSize - 4) * sizeof(char)) != (uiSize - 4) * sizeof(char)) {
			snprintf(szInfo, sizeof(szInfo), "error: file(%s) new %d char failed2\n", strString.c_str(), uiSize);
			CWriteLog::GetInstance().WriteLog(szInfo);
			close(iFd);
			SAFE_DELETE_MUL(pChineseString);
			return false;
		}
		close(iFd);
	}
	return true;
}
//创建树结构
bool CIndexTreeInfo::CreateSearchTree(const int iSelectId, CTreeNode * & pTreeNodeRoot, CTSTPat & clsTSTPat, CSimpleToSpell & csToSpell)
{
    clsTSTPat.init(1024, 0x30000);
    unsigned int uiLoop;
    bool bRet = true;

	CReadBigFile fdR;
	string strPath = m_strDataPath + "/" + m_strTaskName + "/";
	string strFileName = strPath + m_pSysConfigSet->GetFileName(iSelectId);
	fdR.Open(strFileName.c_str());
	char *p;
	int iFileLines = -1;
	char szBody[10240];
	int iKeyField = 1;
	int iPreField = 0;
	int iWeiField = 2;
	if(iSelectId == SELECT_TREE_CHINESE) {
		iKeyField = 0;
		iPreField = 1;
		iWeiField = 3;
	}
	int iWithMix = HasMixSearch(iSelectId);
	//拼音/扩展拼音的获取汉字使用
	XHChineseData * pXHCD = NULL;
	char *pChineseString = NULL;
	if(iWithMix) {
		GetChineseDataAndString(iSelectId, pXHCD, pChineseString);
	}
	//获取影射数据 -->>所有的扩展使用[因为扩展本身的nodedata在非扩展里面，自己本身只有一个影射关系]
	IdToId * pstMapData = NULL;
	bool bCompress = GetCompressMapping(iSelectId, pstMapData);

	unsigned int uiLines = -1;
	while(fdR.GetLine(p)) {
		uiLines++;
		vector<string> vecSplit;
		vector<string> vecSplitBody;
		XHStrUtils::StrTokenizeGBK(vecSplit, p, "|");
		XHStrUtils::StrTokenizeGBK(vecSplitBody, vecSplit[0], ",");
        char *pBufData;
		if(vecSplitBody.size() < iWeiField + 1) {
			printf("error:(exit)line:%s; field:%d[%s %d]\n", p, vecSplitBody.size(), __FILE__, __LINE__);
			exit(-1);
		}
        string strKeyWhole = vecSplitBody[iPreField] + vecSplitBody[iKeyField];
		if(m_iPrefixLen == -1) {
			m_iPrefixLen = vecSplitBody[iPreField].size();
		}
        pBufData = new char[strKeyWhole.size()+1];
        if(!pBufData) {
            printf("new space error[%s %d]\n", __FILE__, __LINE__);
            bRet = false;
            break;
        }
        strncpy(pBufData, strKeyWhole.c_str(), strKeyWhole.size());
        pBufData[strKeyWhole.size()] = 0;
        CLeafNode *pLeafNode;
        pLeafNode = new CLeafNode;
        if(!pLeafNode) {
            printf("new space error[%s %d]\n", __FILE__, __LINE__);
            bRet = false;
            break;
        }
		//混合搜索使用
		if(iWithMix) {
			if(iSelectId == SELECT_TREE_SPELL || iSelectId == SELECT_TREE_EX_SPELL) {
				//拼音查询和扩展的拼音查询
				//为了速度拼音查询/扩展中间查询利用自己对应的汉字词表来查找当前拼音对应的汉字
				//读取时,记录全拼对应的汉字词典的数组下标,然后读取汉字nodedata数据,这样就能找到汉字所对应的内容在chinesestring的位置
				if(pChineseString && pXHCD) {
					if(vecSplit.size() > 1) {
						vector<string> vecSplitExtra;
						XHStrUtils::StrTokenize(vecSplitExtra, vecSplit[1], ",");
						int kLoop;
						for(kLoop = 0; kLoop < vecSplitExtra.size(); kLoop++) {
							unsigned uiChinesePos = atoi(vecSplitExtra[kLoop].c_str());
							char *pChinese = pChineseString + pXHCD[uiChinesePos].uiStringPos;
							csToSpell.AddChineseSpell_2(vecSplitBody[iPreField], vecSplitBody[iKeyField], pChinese);
						}
					}
				} else {
					printf("error: chinese or nodedata is empty[%s %d]\n", __FILE__, __LINE__);
					exit(-1);
				}
			} else if(vecSplit.size() > 2) {
				vector<string> vecSplitMix;
				XHStrUtils::StrTokenizeGBK(vecSplitMix, vecSplit[2], ",");
				if(vecSplitMix.size() > 0) {
					if(iWithMix == 1){ //简拼所有
						csToSpell.AddSpell(vecSplitBody[iPreField], vecSplitMix[0]);
					} else { //中间全拼扩展中间全拼
						//csToSpell.AddSpell(vecSplitBody[iPreField], vecSplitMix[0]);
						for(int kLoop = 0; kLoop < vecSplitMix.size(); kLoop++) {
							csToSpell.AddChineseSpell_2(vecSplitBody[iPreField], vecSplitBody[iKeyField], vecSplitMix[kLoop]);
						}
					}
				}
			}
		}
        pLeafNode->set_key(pBufData,strKeyWhole.size());
		if(bCompress) {
			pLeafNode->setId(pstMapData[uiLines].uiTo);
		} else {
			pLeafNode->setId(uiLines);
		}
		int iWTmp = atoi(vecSplitBody[iWeiField].c_str());
		if(iWTmp > 0xffff) {
			iWTmp = 0xffff;
		} else if(iWTmp < 0) {
			iWTmp = 0;
		}
        pLeafNode->usWeight = iWTmp;
        clsTSTPat.insert(pTreeNodeRoot,pLeafNode);
    }
	SAFE_DELETE_MUL(pXHCD);
	SAFE_DELETE_MUL(pChineseString);
	SAFE_DELETE_MUL(pstMapData);
	return bRet;
}

//--->>>说实话,这个函数特别难调,为了压缩的能更好一些,改了好多次.可能还会存在bug,需要详细的写个文档,否则我保证以后我自己也会看不懂
//填充混合查询相关字段
//首先要找到节点对应的字符串的内容,如果一个节点没有pitem_,那么是找到的对应的字符串的,这里的做法是,如果没有pitem_那么找他的mid,直到找到为止.选取的字符串再采用截取的方法获得 
//如果当前节点是父亲的直接指向的孩子,那么他承担的责任要比他的兄弟多.当有多个自己压缩时,压缩的情况保存在这里
bool CIndexTreeInfo::GetMixSearchContext(CSimpleToSpell &csToSpell, int & iFd, XHRbNode &xhRbNode, CTreeNode * & clsTreeNode, CTreeNode * const & clsNodeParent, unsigned int & uiSSLen, const int iMixSearch)
{
	bool bRet = true;
	///////////////////////---------------->>>这里设计的不是很好
	//if(clsTreeNode->ucolor_ & 1) 
	char szMixRes[102400];
	if(iFd != -1 && xhRbNode.cPosDif >= m_iPrefixLen + CONST_MIXSEARCH_HEAD_LEN_MIN 
			&& xhRbNode.cPosDif < CONST_MIXSEARCH_HEAD_LEN_MAX && clsNodeParent) { //
		//查找对应的字符串
		CTreeNode * pclsTreeNodeTmp = clsTreeNode;
		int iParentPosDif =  clsNodeParent->sbyte_index_;
		int iLoop;
		while(pclsTreeNodeTmp) {
			if(pclsTreeNodeTmp->pitem_) {
				char * pStr = pclsTreeNodeTmp->pitem_->pkey_;
				string strK2;
				string strP = "";
				for(iLoop = 0; iLoop < m_iPrefixLen; iLoop++) strP += pStr[iLoop];
				strK2 = pStr + m_iPrefixLen;
				string strK = strK2; //.substr(0, clsTreeNode->sbyte_index_ - strP.size());
				int iCompressNode = 0;
				int iFrom = clsNodeParent->sbyte_index_ - strP.size() + 2;
				int iTo = clsTreeNode->sbyte_index_ - clsNodeParent->sbyte_index_ + iFrom;
				if(clsNodeParent->pmiddle_ != clsTreeNode) { //中间节点,如果不是中间节点可以最存储最后一个字节不同的结果既可
					//printf("parent-----hit\n");
					iFrom = iTo - 1;
				}
				if(xhRbNode.cByteDif == 0) {
					iTo--;
				}
				int iLoop;
				//mark bug:--->>>>>>>>>>>>>>>>>好的,这里的确还有bug
				//并不是对于所有的"汉字拼音"都能很好的转换,获取结果,但是临时先这样吧,如果获取不到结果,也只有靠拼音获取了.(不敢做的太多,会增大内存)
				//后面再想办法解决吧---------------------------------------------------------------------------------------------------------------todo 仍然有bug
				//e.g: 一个节点代表了xbhx,当前的不同字节为:h，但是其没有中间孩子，但是有数据，数据则为hx的信息，这时候匹配时会匹配成功
				//不能只查询xbh可以，也要让xbhx能查询到
				if(clsTreeNode->pmiddle_ == NULL && clsTreeNode->pitem_) { //如果middle,但是有数据部分，那么需要把其数据部分所能代表的所有的字符都给出相应的全拼组合
					//这里的设计思路是这样的
					//如果获取对应的字符串经过切分之后,结果个数为1个,那么也就是说后面的结果都是相同的,因此就没有必要保存多次了,所以只保留一个最长的就可以
					string strKTmp = strK.substr(0, iFrom); //当前节点第一个不同的字符
					string strKNew = csToSpell.GetSpellFromSimple(strP, strKTmp, iMixSearch);
					if(strKNew.find('|') == string::npos) {
						for(iLoop = strK2.size(); iLoop >= iFrom; iLoop--) {
							//如果要进行压缩,首先保证剪枝部分可以出现结果
							string strKTmp = strK.substr(0, iLoop); //当前节点第一个不同的字符
							string strKNew = csToSpell.GetSpellFromSimple(strP, strKTmp, iMixSearch);
							if(strKNew != "") {
								break;
							}
						}
						iFrom = iLoop;
						//iFrom = strK2.size();
					}
					iTo = strK2.size() + 1;
				}
				//printf("parent[%d %c]; node:[%d %c](ifrom:%d; ito:%d)\n", clsNodeParent->sbyte_index_, clsNodeParent->cbyte_, clsTreeNode->sbyte_index_, clsTreeNode->cbyte_, iFrom, iTo);
				vector<pair<int,string> > vecMix;
				for(iLoop = iFrom; iLoop < iTo; iLoop++) {
					string strKTmp = strK.substr(0, iLoop); //当前节点第一个不同的字符
					string strKNew = csToSpell.GetSpellFromSimple(strP, strKTmp, iMixSearch);
					//浪费内存的例子 
					//iLoop:6; strktmp:AiShan; strknew:爱上|艾尚; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:7; strktmp:AiShang; strknew:爱上|艾尚; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:8; strktmp:AiShangM; strknew:艾尚美; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:9; strktmp:AiShangMe; strknew:艾尚美; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:10; strktmp:AiShangMei; strknew:艾尚美; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:11; strktmp:AiShangMeiC; strknew:艾尚美彩; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:12; strktmp:AiShangMeiCa; strknew:艾尚美彩; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:13; strktmp:AiShangMeiCai; strknew:艾尚美彩; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:14; strktmp:AiShangMeiCaiZ; strknew:艾尚美彩妆; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:15; strktmp:AiShangMeiCaiZh; strknew:艾尚美彩妆; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:16; strktmp:AiShangMeiCaiZhu; strknew:艾尚美彩妆; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:17; strktmp:AiShangMeiCaiZhua; strknew:艾尚美彩妆; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:18; strktmp:AiShangMeiCaiZhuan; strknew:艾尚美彩妆; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:19; strktmp:AiShangMeiCaiZhuang; strknew:艾尚美彩妆; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:20; strktmp:AiShangMeiCaiZhuangZ; strknew:艾尚美彩妆造; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:21; strktmp:AiShangMeiCaiZhuangZa; strknew:艾尚美彩妆造; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:22; strktmp:AiShangMeiCaiZhuangZao; strknew:艾尚美彩妆造; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:23; strktmp:AiShangMeiCaiZhuangZaoX; strknew:艾尚美彩妆造型; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:24; strktmp:AiShangMeiCaiZhuangZaoXi; strknew:艾尚美彩妆造型; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:25; strktmp:AiShangMeiCaiZhuangZaoXin; strknew:艾尚美彩妆造型; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:26; strktmp:AiShangMeiCaiZhuangZaoXing; strknew:艾尚美彩妆造型; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:27; strktmp:AiShangMeiCaiZhuangZaoXingM; strknew:艾尚美彩妆造型美; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:28; strktmp:AiShangMeiCaiZhuangZaoXingMe; strknew:艾尚美彩妆造型美; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:29; strktmp:AiShangMeiCaiZhuangZaoXingMei; strknew:艾尚美彩妆造型美; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:30; strktmp:AiShangMeiCaiZhuangZaoXingMeiJ; strknew:艾尚美彩妆造型美甲; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:31; strktmp:AiShangMeiCaiZhuangZaoXingMeiJi; strknew:艾尚美彩妆造型美甲; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:32; strktmp:AiShangMeiCaiZhuangZaoXingMeiJia; strknew:艾尚美彩妆造型美甲; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:33; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaS; strknew:艾尚美彩妆造型美甲沙; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:34; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaSh; strknew:艾尚美彩妆造型美甲沙; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:35; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaSha; strknew:艾尚美彩妆造型美甲沙; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:36; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaShaL; strknew:艾尚美彩妆造型美甲沙龙; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:37; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaShaLo; strknew:艾尚美彩妆造型美甲沙龙; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:38; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaShaLon; strknew:艾尚美彩妆造型美甲沙龙; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:39; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; strknew:艾尚美彩妆造型美甲沙龙; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
#if 1
					if(strKNew.find('|') == string::npos) { //如果从当前字节往后都是一个字符串对应,那么就直接使用最长的那个字符串吧,节省空间
						if(iLoop != iTo - 1) {
							int iLoopT = iLoop;
							string strKTmp2, strKNew2;
							//这里是为了处理含有字母的情况:圣馨大地家园c座
							for(iLoop = iTo - 1; iLoop > iLoopT; iLoop--) { //从最后开始找能用的,直到找到第一个可以用的作为标准
								strKTmp2 = strK.substr(0, iLoop); //当前节点第一个不同的字符
								strKNew2 = csToSpell.GetSpellFromSimple(strP, strKTmp2, iMixSearch);
								if(strKNew2 != "") {
									break;
								}
							}
							//printf("ifrom->%d; iTo->%d; iLoop->%d; iLoopT->%d; strK->%s; strKTmp2->%s; strKNew2->%s\n", iFrom, iTo, iLoop, iLoopT, strK.c_str(), strKTmp2.c_str(), strKNew2.c_str());
							if(iLoop == iLoopT) { //如果从当前往后的全部不能用,只有当前能用
							} else { //后面还存在可以用的
								strKTmp = strKTmp2;
								strKNew = strKNew2;
								iTo = iLoop; //执行完这次之后就结束了
							}
							//printf("save--->>>>iLoop:%d; strktmp:%s; strknew:%s; from:%d; to:%d; strK:%s; parent diff:%d; node diff:%d\n", iLoop, strKTmp.c_str(), strKNew.c_str(), iFrom, iTo, strK.c_str(), clsNodeParent->sbyte_index_, clsTreeNode->sbyte_index_);
						}
					}
#endif
					//printf("iLoop:%d; strktmp:%s; strknew:%s; from:%d; to:%d; strK:%s; parent diff:%d; node diff:%d\n", iLoop, strKTmp.c_str(), strKNew.c_str(), iFrom, iTo, strK.c_str(), clsNodeParent->sbyte_index_, clsTreeNode->sbyte_index_);
					if(strKNew == "") { //短的字符串获取失败,长的字符串更不会成功
#if _WITH_DEBUG_
						char szCxt[1024];
						snprintf(szCxt, sizeof(szCxt), "iLoop:%d; strktmp:%s; strknew:%s; strK:%s\n", iLoop, strKTmp.c_str(), strKNew.c_str(), strK.c_str());
						CWriteLog::GetInstance().WriteLog(szCxt);
#endif
						break;
					}
					if(iMixSearch == 1) { //简拼的混合是要进行压缩的
						vecMix.push_back(make_pair(iLoop, strKNew));
					} else { //全拼的混合 -->>>还待改进
						//--->>这里的描述还是使用之前的对拼音的描述,事实上这里只会处理汉字了,但是原理是一样的,描述还没有来得及改
						//uiSpellSelf 最高位为标志位，最高位为1则代表该节点为含有多个结果的压缩节点
						//CharString~CharString~CharString\0
						//  |   |   |                     \|/
						//  |   |  \|/                    结尾符号
						//  |  \|/ 多个结果之间的分割符(为什么使用~，因为~的ascii码比较大) --->>好吧,我承认,我随便选的
						// \|/ GetSpellFromSimple返回的内容
						// 当前为几个简拼字母的内容
						if(!(xhRbNode.usHua & XH_TREE_HUA_INFO_SSELF)) { //第一次
							xhRbNode.usHua |= XH_TREE_HUA_INFO_SSELF;
							//最高位标志位，标志该节点为含有多个结果的节点
							xhRbNode.uiSpellSelf = uiSSLen | (1<<31);
							//printf("xhRbNode.uiSpellSelf-->%x; uiSSLen:%x\n", xhRbNode.uiSpellSelf, uiSSLen);
							unsigned char ucT = strKTmp.size();
							write(iFd, &ucT, 1);
							write(iFd, strKNew.c_str(), strKNew.size());
							uiSSLen += strKNew.size() + 1;
						} else {
							write(iFd, "~", 1);
							unsigned char ucT = strKTmp.size();
							write(iFd, &ucT, 1);
							write(iFd, strKNew.c_str(), strKNew.size());
							uiSSLen += strKNew.size() + 2;
						}
					}
				}
				//printf("node begin--------------------------------------------------------->>>>end\n");
				if(iMixSearch == 1) {
					//简拼全拼混合查询
					//首先上面多个char之间有空格是为了显示好看,事实上没有
					//压缩方法:char char char ... [ char char ...     ] [0xfe      char char char char ...] 0xff
					//          |  |-> n个字节<-| |->多个n字节的情况<-|  \|/       |->标准一部分<---------|   |
					//         \|/     /|\                            树压缩节点                             \|/
					// 当前处理简拼     |                          最上面那个孩子                       最后结束符号
					// 的长度n          |                          包含多个字节的
					//       符合条件的全拼除去                    信息的分隔符
					//       首字母之后后面内容
					//       的压缩编码
					//        |->标准一部分<--------------------------|
					int iMixRetSize;
					if(CCompressPinyin::GetInstance().PacketMixSearch(vecMix, szMixRes, sizeof(szMixRes) / sizeof(szMixRes[0]), iMixRetSize)) {
						xhRbNode.usHua |= XH_TREE_HUA_INFO_SSELF;
						//xhRbNode.uiSpellSelf = uiSSLen | (1<<31);
						xhRbNode.uiSpellSelf = uiSSLen;
						write(iFd, szMixRes, iMixRetSize);
						uiSSLen += iMixRetSize;
					} else {
						int a=1;
					}
				} else { //全拼的混合
					if(xhRbNode.usHua & XH_TREE_HUA_INFO_SSELF) { //可能存在找不找的情况，既然没有找到，也不再多浪费一个字节了
						write(iFd, "\0", 1);
						uiSSLen += 1;
					}
				}
				break;
			}
			pclsTreeNodeTmp = pclsTreeNodeTmp->pmiddle_;
		}
	}
	///////////////////////---------------->>>这里设计的不是很好----->>.还需要再细化一下
	return bRet;
}
//给树的各个节点进行编号,并且记录最辉煌的子孙(查看孩子节点中weight最大的那个,记录下来)
//按照后续遍历的方式进行编号
//为了减少遍历次数,编号是指写到内存中时的地址位置,不是连续的数字
//uiMemPosCanUse: 处理节点时,可以使用的起始地址,因为是后续遍历,如果有子节点,首先从这里开始
//uiMemPosUsing:  实际使用的地址,当前节点放到内存时的地址
//uiMemUseTotal:  总共用了多少了,接下来,再使用就从这里开始
bool CIndexTreeInfo::Do_WriteTreeNode(int &iFdSS, CSimpleToSpell &csToSpell, CTreeNode * const & clsTreeRoot, const unsigned int & uiMemPosCanUse, unsigned int & uiMemPosUsing, unsigned int & uiMemUseTotal, int & iFileFd, CTreeNode * & clsTreeNode, unsigned int & uiSSLen, CTreeNode * const & clsNodeParent, const int iMixSearch)
{
    bool bRet = true;
    
    XHRbNode xhRbNodeNew;
    unsigned int uiMemTmp;
    unsigned int uiMemNode;
    unsigned short usWMid = 0, usWLeft = 0, usWRight = 0, usWMy = 0;

    xhRbNodeNew.cByteDif = clsTreeNode->cbyte_;
    xhRbNodeNew.cPosDif  = clsTreeNode->sbyte_index_;
	xhRbNodeNew.usHua = 0;
    if(clsTreeNode->pleft_) {
        xhRbNodeNew.usHua |= XH_TREE_HUA_INFO_LEFT;
        uiMemTmp = uiMemUseTotal;
        Do_WriteTreeNode(iFdSS, csToSpell, clsTreeRoot, uiMemTmp, uiMemNode, uiMemUseTotal, iFileFd, clsTreeNode->pleft_, uiSSLen, clsTreeNode, iMixSearch);
        xhRbNodeNew.uiLeft = uiMemNode;
		usWLeft = clsTreeNode->pleft_->usWeight;
    }
    if(clsTreeNode->pmiddle_) {
        xhRbNodeNew.usHua |= XH_TREE_HUA_INFO_MID;
        uiMemTmp = uiMemUseTotal;
        Do_WriteTreeNode(iFdSS, csToSpell, clsTreeRoot, uiMemTmp, uiMemNode, uiMemUseTotal, iFileFd, clsTreeNode->pmiddle_, uiSSLen, clsTreeNode, iMixSearch);
        xhRbNodeNew.uiMid = uiMemNode;
        usWMid = clsTreeNode->pmiddle_->usWeight;
    }
    if(clsTreeNode->pright_) {
        xhRbNodeNew.usHua |= XH_TREE_HUA_INFO_RIGHT;
        uiMemTmp = uiMemUseTotal;
        Do_WriteTreeNode(iFdSS, csToSpell, clsTreeRoot, uiMemTmp, uiMemNode, uiMemUseTotal, iFileFd, clsTreeNode->pright_, uiSSLen, clsTreeNode, iMixSearch);
        xhRbNodeNew.uiRight = uiMemNode;
		usWRight = clsTreeNode->pright_->usWeight;
    }
	usWMid = usWMid > usWLeft ? usWMid : usWLeft;
	usWMid = usWMid > usWRight ? usWMid : usWRight;

    if(clsTreeNode->pitem_) {
        xhRbNodeNew.usHua |= XH_TREE_HUA_INFO_WIFE;
        xhRbNodeNew.uiWife = clsTreeNode->pitem_->elemid * sizeof(XHSpellData) + 4;
        usWMy = clsTreeNode->pitem_->usWeight;
        usWMy = usWMy > MAX_WEIGHT_CHINESE ? MAX_WEIGHT_CHINESE : usWMy;
    }
	if(iFdSS != -1) {
		if(uiSSLen > (1<<31)) {
			char szCxt[1024];
			snprintf(szCxt, sizeof(szCxt), "error: the self simple string is too big[%s %d]\n", __FILE__, __LINE__);
			CWriteLog::GetInstance().WriteLog(szCxt);
		} else {
			GetMixSearchContext(csToSpell, iFdSS, xhRbNodeNew, clsTreeNode, clsNodeParent, uiSSLen, iMixSearch);
		}
	}
	xhRbNodeNew.usWPeer = clsTreeNode->bHavePeer;
    xhRbNodeNew.usWeight = usWMid > usWMy ? usWMid : usWMy; //光宗耀祖系列
	clsTreeNode->usWeight = xhRbNodeNew.usWeight; //修改原始树中节点的权重

#if _WITH_DEBUG_ //打印树内容,调试
    {
        char szLog[1024];
        snprintf(szLog, sizeof(szLog), "--------->>node");
		CWriteLog::GetInstance().WriteLog(szLog);
        snprintf(szLog, sizeof(szLog), "         >>origsize:%lu| composesize:%u", sizeof(xhRbNodeNew), xhRbNodeNew.GetComposeSize());  CWriteLog::GetInstance().WriteLog(szLog);
        snprintf(szLog, sizeof(szLog), "         >>UseMemory:%u", uiMemUseTotal); CWriteLog::GetInstance().WriteLog(szLog);
        snprintf(szLog, sizeof(szLog), "         >>left:%u; right:%u; usWMy:%u; usWeight:%u", usWLeft, usWRight, usWMy, clsTreeNode->usWeight); CWriteLog::GetInstance().WriteLog(szLog);
    }
#endif

    xhRbNodeNew.ComposeCut(); //压缩
    int iComposeSize = xhRbNodeNew.GetComposeSize();

    if(clsTreeRoot == clsTreeNode) { //是根节点的时候,不再多申请空间
        lseek(iFileFd, SEEK_SET, 0);
        write(iFileFd, &xhRbNodeNew, 4); //随便填充4个字节
        write(iFileFd, &xhRbNodeNew, iComposeSize);
    } else {
        write(iFileFd, &xhRbNodeNew, iComposeSize);
        uiMemPosUsing = uiMemUseTotal; //当前节点的首地址
        uiMemUseTotal += iComposeSize;
    }
    return bRet;
}
//0->不存在
//1->简拼
//2->全拼
int CIndexTreeInfo::HasMixSearch(const int iSelectId)
{
	if(m_pSysConfigSet->GetInt(PARA_CONFIG_withmixsearch)) {
		if(iSelectId == SELECT_TREE_SIMPLE && m_bDictNeedCreate[SELECT_TREE_SPELL]) {
			return 1;
		} else if(iSelectId == SELECT_TREE_EX_SIMPLE && m_bDictNeedCreate[SELECT_TREE_EX_SPELL]) {
			return 1;
		} else if(iSelectId == SELECT_TREE_CENTER_SIMPLE && m_bDictNeedCreate[SELECT_TREE_CENTER_SPELL]) {
			return 1;
		} else if(iSelectId == SELECT_TREE_EX_CENTER_SIMPLE && m_bDictNeedCreate[SELECT_TREE_EX_CENTER_SPELL]) {
			return 1;
		} else if(iSelectId == SELECT_TREE_SPELL && m_bDictNeedCreate[SELECT_TREE_CHINESE]) {
			return 2;
		} else if(iSelectId == SELECT_TREE_EX_SPELL && m_bDictNeedCreate[SELECT_TREE_EX_CHINESE]) {
			return 2;
		} else if(iSelectId == SELECT_TREE_CENTER_SPELL && m_bDictNeedCreate[SELECT_TREE_CENTER_CHINESE]) {
			return 2;
		} else if(iSelectId == SELECT_TREE_EX_CENTER_SPELL && m_bDictNeedCreate[SELECT_TREE_EX_CENTER_CHINESE]) {
			return 2;
		} else {
			return 0;
		} 
	}
	return 0;
}
bool CIndexTreeInfo::WriteTreeNode(const int iSelectId, CTreeNode * & clsTreeNode, CSimpleToSpell & csToSpell)
{
	string strPath = m_strDataPath + "/" + m_strTaskName + "/";
	string strSelect = strPath + m_pSysConfigSet->GetFileName(iSelectId);
	string strTreeNode = strSelect + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_treenode);
    bool bRet = true;
    int iFdOut = open(strTreeNode.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    int iFdMS = -1;
	string strMixSearch = strSelect + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_mixsearch);
	int iMixSearch = 0;
	if((iMixSearch = HasMixSearch(iSelectId)) != 0) {
		iFdMS = open(strMixSearch.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	}
    unsigned int uiMemBegin = 4;
    unsigned int uiNodeBegin;
    unsigned int uiUseTotal = 4;
    char szLog[1024];
    do {
        if(iFdOut == -1/* || iFdMS == -1*/) {
            snprintf(szLog, sizeof(szLog), "open or create file error[%s|%s][%s %d]\n", strTreeNode.c_str(), strMixSearch.c_str(), __FILE__, __LINE__);
            bRet = false;
            CWriteLog::GetInstance().WriteLog(szLog);
            goto WriteTreeNodeExit;
        }
        write(iFdOut, &uiUseTotal, 4); //写4个字节,过会用'扩展信息的长度'覆盖

		if(!clsTreeNode) break;
        //---------------->>>把根节点放到第一的位置==========begin
        int iTreeRootSize = (clsTreeNode->pleft_ ? 1 : 0) + (clsTreeNode->pright_ ? 1 : 0) + (clsTreeNode->pmiddle_ ? 1 : 0) + (clsTreeNode->pitem_ ? 1 : 0);
        iTreeRootSize = sizeof(XHRbNode) - sizeof(unsigned int) * ( 4 - iTreeRootSize);
        write(iFdOut, szLog, iTreeRootSize);
        uiMemBegin += iTreeRootSize;
        uiUseTotal += iTreeRootSize;
        //========================================end
        //填充simpleself的前四个字节->first
        unsigned int uiSSLen = 4;
		if(iFdMS != -1) {
			write(iFdMS, &uiSSLen, 4);
		}
        //填充simpleself的前四个字节--->over

        Do_WriteTreeNode(iFdMS, csToSpell, clsTreeNode, uiMemBegin, uiNodeBegin, uiUseTotal, iFdOut, clsTreeNode, uiSSLen, NULL, iMixSearch);

        //填充simpleself的前四个字节->second
		if(iFdMS != -1) {
			lseek(iFdMS, SEEK_SET, 0);
			write(iFdMS, &uiSSLen, 4);
		}
        //填充simpleself的前四个字节--->over

        //printf("================%d, begin:%d, total:%d\n", iTreeRootSize, uiMemBegin, uiUseTotal);
        lseek(iFdOut, SEEK_SET, 0);
        write(iFdOut, &uiUseTotal, 4); //old 没有把根节点提前时
        //write(iFdOut, &uiNodeBegin, 4);
        snprintf(szLog, sizeof(szLog), "WriteTreeNode : uiUseMem->%u\n", uiUseTotal);
        CWriteLog::GetInstance().WriteLog(szLog);
    } while(0);

	if(iFdMS != -1) {
		close(iFdMS);
	}
    close(iFdOut);
WriteTreeNodeExit:
    return bRet;
}

bool CIndexTreeInfo::BuildTrees(const int iSelectId)
{
	CTreeNode * pTreeNodeRoot = NULL;
    CTSTPat    clsTSTPat;
	CSimpleToSpell csToSpell;
	bool bRet = true;
	do {
		int iWithMix = HasMixSearch(iSelectId);
		bRet = CreateSearchTree(iSelectId, pTreeNodeRoot, clsTSTPat, csToSpell);
		if(iWithMix) {
			csToSpell.Sort();
		}
		if(bRet == false) break;
		//只有拼音树才有peer信息
		if(iSelectId == SELECT_TREE_SPELL || iSelectId == SELECT_TREE_CENTER_SPELL || iSelectId == SELECT_TREE_EX_SPELL || iSelectId == SELECT_TREE_EX_CENTER_SPELL) {
			bRet = MarkHavePeerInfo(pTreeNodeRoot);
			if(bRet == false) break;
		}
		bRet = WriteTreeNode(iSelectId, pTreeNodeRoot, csToSpell);
		if(bRet == false) break;
	}while(0);

	//释放空间
	if(pTreeNodeRoot) {
		FreeTreeNodeData(pTreeNodeRoot);
	}
	return bRet;
}

int CIndexTreeInfo::Process()
{
	int iLoop;
	char szLog[256];
	for(iLoop = 0; iLoop < DICT_MAX_SIZE; iLoop++) {
		if(m_bDictNeedCreate[iLoop]) {
			BuildTrees(iLoop);
			snprintf(szLog, sizeof(szLog), "CIndexTreeInfo::Process over:%d\n", iLoop);
			CWriteLog::GetInstance().WriteLog(szLog);
		}
	}
	return 0;
}
