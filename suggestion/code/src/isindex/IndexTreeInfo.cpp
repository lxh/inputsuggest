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
	//��ʼ��Ҫ���ɵ�dict
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

//�ͷŵ����������ڵ�ʱ,������ڴ�
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
//����һ���ڵ�����е��ֵܽڵ�(left,right)
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
//������к���,�������к��ӵ��ֵ��ǣ��к��ӵ����Һ��ӵĵݹ飩�����,Ȼ���ж��ڲ�ͬ�ֽ�λ����ͬ�ĵط���ֵ�Ƿ���ڶ��
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
                    if(iIterPart2 != iIterAll->second.end()) { //�ҵ�
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
//��ȡ���ֵ�nodedata,chinesestring
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
//�������ṹ
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
	//ƴ��/��չƴ���Ļ�ȡ����ʹ��
	XHChineseData * pXHCD = NULL;
	char *pChineseString = NULL;
	if(iWithMix) {
		GetChineseDataAndString(iSelectId, pXHCD, pChineseString);
	}
	//��ȡӰ������ -->>���е���չʹ��[��Ϊ��չ�����nodedata�ڷ���չ���棬�Լ�����ֻ��һ��Ӱ���ϵ]
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
		//�������ʹ��
		if(iWithMix) {
			if(iSelectId == SELECT_TREE_SPELL || iSelectId == SELECT_TREE_EX_SPELL) {
				//ƴ����ѯ����չ��ƴ����ѯ
				//Ϊ���ٶ�ƴ����ѯ/��չ�м��ѯ�����Լ���Ӧ�ĺ��ִʱ������ҵ�ǰƴ����Ӧ�ĺ���
				//��ȡʱ,��¼ȫƴ��Ӧ�ĺ��ִʵ�������±�,Ȼ���ȡ����nodedata����,���������ҵ���������Ӧ��������chinesestring��λ��
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
					if(iWithMix == 1){ //��ƴ����
						csToSpell.AddSpell(vecSplitBody[iPreField], vecSplitMix[0]);
					} else { //�м�ȫƴ��չ�м�ȫƴ
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

//--->>>˵ʵ��,��������ر��ѵ�,Ϊ��ѹ�����ܸ���һЩ,���˺ö��.���ܻ������bug,��Ҫ��ϸ��д���ĵ�,�����ұ�֤�Ժ����Լ�Ҳ�ῴ����
//����ϲ�ѯ����ֶ�
//����Ҫ�ҵ��ڵ��Ӧ���ַ���������,���һ���ڵ�û��pitem_,��ô���ҵ��Ķ�Ӧ���ַ�����,�����������,���û��pitem_��ô������mid,ֱ���ҵ�Ϊֹ.ѡȡ���ַ����ٲ��ý�ȡ�ķ������ 
//�����ǰ�ڵ��Ǹ��׵�ֱ��ָ��ĺ���,��ô���е�������Ҫ�������ֵܶ�.���ж���Լ�ѹ��ʱ,ѹ�����������������
bool CIndexTreeInfo::GetMixSearchContext(CSimpleToSpell &csToSpell, int & iFd, XHRbNode &xhRbNode, CTreeNode * & clsTreeNode, CTreeNode * const & clsNodeParent, unsigned int & uiSSLen, const int iMixSearch)
{
	bool bRet = true;
	///////////////////////---------------->>>������ƵĲ��Ǻܺ�
	//if(clsTreeNode->ucolor_ & 1) 
	char szMixRes[102400];
	if(iFd != -1 && xhRbNode.cPosDif >= m_iPrefixLen + CONST_MIXSEARCH_HEAD_LEN_MIN 
			&& xhRbNode.cPosDif < CONST_MIXSEARCH_HEAD_LEN_MAX && clsNodeParent) { //
		//���Ҷ�Ӧ���ַ���
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
				if(clsNodeParent->pmiddle_ != clsTreeNode) { //�м�ڵ�,��������м�ڵ������洢���һ���ֽڲ�ͬ�Ľ���ȿ�
					//printf("parent-----hit\n");
					iFrom = iTo - 1;
				}
				if(xhRbNode.cByteDif == 0) {
					iTo--;
				}
				int iLoop;
				//mark bug:--->>>>>>>>>>>>>>>>>�õ�,�����ȷ����bug
				//�����Ƕ������е�"����ƴ��"���ܺܺõ�ת��,��ȡ���,������ʱ��������,�����ȡ�������,Ҳֻ�п�ƴ����ȡ��.(��������̫��,�������ڴ�)
				//��������취�����---------------------------------------------------------------------------------------------------------------todo ��Ȼ��bug
				//e.g: һ���ڵ������xbhx,��ǰ�Ĳ�ͬ�ֽ�Ϊ:h��������û���м亢�ӣ����������ݣ�������Ϊhx����Ϣ����ʱ��ƥ��ʱ��ƥ��ɹ�
				//����ֻ��ѯxbh���ԣ�ҲҪ��xbhx�ܲ�ѯ��
				if(clsTreeNode->pmiddle_ == NULL && clsTreeNode->pitem_) { //���middle,���������ݲ��֣���ô��Ҫ�������ݲ������ܴ�������е��ַ���������Ӧ��ȫƴ���
					//��������˼·��������
					//�����ȡ��Ӧ���ַ��������з�֮��,�������Ϊ1��,��ôҲ����˵����Ľ��������ͬ��,��˾�û�б�Ҫ��������,����ֻ����һ����ľͿ���
					string strKTmp = strK.substr(0, iFrom); //��ǰ�ڵ��һ����ͬ���ַ�
					string strKNew = csToSpell.GetSpellFromSimple(strP, strKTmp, iMixSearch);
					if(strKNew.find('|') == string::npos) {
						for(iLoop = strK2.size(); iLoop >= iFrom; iLoop--) {
							//���Ҫ����ѹ��,���ȱ�֤��֦���ֿ��Գ��ֽ��
							string strKTmp = strK.substr(0, iLoop); //��ǰ�ڵ��һ����ͬ���ַ�
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
					string strKTmp = strK.substr(0, iLoop); //��ǰ�ڵ��һ����ͬ���ַ�
					string strKNew = csToSpell.GetSpellFromSimple(strP, strKTmp, iMixSearch);
					//�˷��ڴ������ 
					//iLoop:6; strktmp:AiShan; strknew:����|����; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:7; strktmp:AiShang; strknew:����|����; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:8; strktmp:AiShangM; strknew:������; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:9; strktmp:AiShangMe; strknew:������; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:10; strktmp:AiShangMei; strknew:������; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:11; strktmp:AiShangMeiC; strknew:��������; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:12; strktmp:AiShangMeiCa; strknew:��������; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:13; strktmp:AiShangMeiCai; strknew:��������; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:14; strktmp:AiShangMeiCaiZ; strknew:��������ױ; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:15; strktmp:AiShangMeiCaiZh; strknew:��������ױ; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:16; strktmp:AiShangMeiCaiZhu; strknew:��������ױ; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:17; strktmp:AiShangMeiCaiZhua; strknew:��������ױ; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:18; strktmp:AiShangMeiCaiZhuan; strknew:��������ױ; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:19; strktmp:AiShangMeiCaiZhuang; strknew:��������ױ; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:20; strktmp:AiShangMeiCaiZhuangZ; strknew:��������ױ��; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:21; strktmp:AiShangMeiCaiZhuangZa; strknew:��������ױ��; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:22; strktmp:AiShangMeiCaiZhuangZao; strknew:��������ױ��; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:23; strktmp:AiShangMeiCaiZhuangZaoX; strknew:��������ױ����; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:24; strktmp:AiShangMeiCaiZhuangZaoXi; strknew:��������ױ����; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:25; strktmp:AiShangMeiCaiZhuangZaoXin; strknew:��������ױ����; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:26; strktmp:AiShangMeiCaiZhuangZaoXing; strknew:��������ױ����; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:27; strktmp:AiShangMeiCaiZhuangZaoXingM; strknew:��������ױ������; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:28; strktmp:AiShangMeiCaiZhuangZaoXingMe; strknew:��������ױ������; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:29; strktmp:AiShangMeiCaiZhuangZaoXingMei; strknew:��������ױ������; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:30; strktmp:AiShangMeiCaiZhuangZaoXingMeiJ; strknew:��������ױ��������; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:31; strktmp:AiShangMeiCaiZhuangZaoXingMeiJi; strknew:��������ױ��������; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:32; strktmp:AiShangMeiCaiZhuangZaoXingMeiJia; strknew:��������ױ��������; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:33; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaS; strknew:��������ױ��������ɳ; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:34; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaSh; strknew:��������ױ��������ɳ; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:35; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaSha; strknew:��������ױ��������ɳ; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:36; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaShaL; strknew:��������ױ��������ɳ��; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:37; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaShaLo; strknew:��������ױ��������ɳ��; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:38; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaShaLon; strknew:��������ױ��������ɳ��; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
					//iLoop:39; strktmp:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; strknew:��������ױ��������ɳ��; from:6; to:40; strK:AiShangMeiCaiZhuangZaoXingMeiJiaShaLong; parent diff:9; node diff:12
#if 1
					if(strKNew.find('|') == string::npos) { //����ӵ�ǰ�ֽ�������һ���ַ�����Ӧ,��ô��ֱ��ʹ������Ǹ��ַ�����,��ʡ�ռ�
						if(iLoop != iTo - 1) {
							int iLoopT = iLoop;
							string strKTmp2, strKNew2;
							//������Ϊ�˴�������ĸ�����:ʥܰ��ؼ�԰c��
							for(iLoop = iTo - 1; iLoop > iLoopT; iLoop--) { //�����ʼ�����õ�,ֱ���ҵ���һ�������õ���Ϊ��׼
								strKTmp2 = strK.substr(0, iLoop); //��ǰ�ڵ��һ����ͬ���ַ�
								strKNew2 = csToSpell.GetSpellFromSimple(strP, strKTmp2, iMixSearch);
								if(strKNew2 != "") {
									break;
								}
							}
							//printf("ifrom->%d; iTo->%d; iLoop->%d; iLoopT->%d; strK->%s; strKTmp2->%s; strKNew2->%s\n", iFrom, iTo, iLoop, iLoopT, strK.c_str(), strKTmp2.c_str(), strKNew2.c_str());
							if(iLoop == iLoopT) { //����ӵ�ǰ�����ȫ��������,ֻ�е�ǰ����
							} else { //���滹���ڿ����õ�
								strKTmp = strKTmp2;
								strKNew = strKNew2;
								iTo = iLoop; //ִ�������֮��ͽ�����
							}
							//printf("save--->>>>iLoop:%d; strktmp:%s; strknew:%s; from:%d; to:%d; strK:%s; parent diff:%d; node diff:%d\n", iLoop, strKTmp.c_str(), strKNew.c_str(), iFrom, iTo, strK.c_str(), clsNodeParent->sbyte_index_, clsTreeNode->sbyte_index_);
						}
					}
#endif
					//printf("iLoop:%d; strktmp:%s; strknew:%s; from:%d; to:%d; strK:%s; parent diff:%d; node diff:%d\n", iLoop, strKTmp.c_str(), strKNew.c_str(), iFrom, iTo, strK.c_str(), clsNodeParent->sbyte_index_, clsTreeNode->sbyte_index_);
					if(strKNew == "") { //�̵��ַ�����ȡʧ��,�����ַ���������ɹ�
#if _WITH_DEBUG_
						char szCxt[1024];
						snprintf(szCxt, sizeof(szCxt), "iLoop:%d; strktmp:%s; strknew:%s; strK:%s\n", iLoop, strKTmp.c_str(), strKNew.c_str(), strK.c_str());
						CWriteLog::GetInstance().WriteLog(szCxt);
#endif
						break;
					}
					if(iMixSearch == 1) { //��ƴ�Ļ����Ҫ����ѹ����
						vecMix.push_back(make_pair(iLoop, strKNew));
					} else { //ȫƴ�Ļ�� -->>>�����Ľ�
						//--->>�������������ʹ��֮ǰ�Ķ�ƴ��������,��ʵ������ֻ�ᴦ������,����ԭ����һ����,������û�����ü���
						//uiSpellSelf ���λΪ��־λ�����λΪ1�����ýڵ�Ϊ���ж�������ѹ���ڵ�
						//CharString~CharString~CharString\0
						//  |   |   |                     \|/
						//  |   |  \|/                    ��β����
						//  |  \|/ ������֮��ķָ��(Ϊʲôʹ��~����Ϊ~��ascii��Ƚϴ�) --->>�ð�,�ҳ���,�����ѡ��
						// \|/ GetSpellFromSimple���ص�����
						// ��ǰΪ������ƴ��ĸ������
						if(!(xhRbNode.usHua & XH_TREE_HUA_INFO_SSELF)) { //��һ��
							xhRbNode.usHua |= XH_TREE_HUA_INFO_SSELF;
							//���λ��־λ����־�ýڵ�Ϊ���ж������Ľڵ�
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
					//��ƴȫƴ��ϲ�ѯ
					//����������char֮���пո���Ϊ����ʾ�ÿ�,��ʵ��û��
					//ѹ������:char char char ... [ char char ...     ] [0xfe      char char char char ...] 0xff
					//          |  |-> n���ֽ�<-| |->���n�ֽڵ����<-|  \|/       |->��׼һ����<---------|   |
					//         \|/     /|\                            ��ѹ���ڵ�                             \|/
					// ��ǰ�����ƴ     |                          �������Ǹ�����                       ����������
					// �ĳ���n          |                          ��������ֽڵ�
					//       ����������ȫƴ��ȥ                    ��Ϣ�ķָ���
					//       ����ĸ֮���������
					//       ��ѹ������
					//        |->��׼һ����<--------------------------|
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
				} else { //ȫƴ�Ļ��
					if(xhRbNode.usHua & XH_TREE_HUA_INFO_SSELF) { //���ܴ����Ҳ��ҵ��������Ȼû���ҵ���Ҳ���ٶ��˷�һ���ֽ���
						write(iFd, "\0", 1);
						uiSSLen += 1;
					}
				}
				break;
			}
			pclsTreeNodeTmp = pclsTreeNodeTmp->pmiddle_;
		}
	}
	///////////////////////---------------->>>������ƵĲ��Ǻܺ�----->>.����Ҫ��ϸ��һ��
	return bRet;
}
//�����ĸ����ڵ���б��,���Ҽ�¼��Ի͵�����(�鿴���ӽڵ���weight�����Ǹ�,��¼����)
//���պ��������ķ�ʽ���б��
//Ϊ�˼��ٱ�������,�����ָд���ڴ���ʱ�ĵ�ַλ��,��������������
//uiMemPosCanUse: ����ڵ�ʱ,����ʹ�õ���ʼ��ַ,��Ϊ�Ǻ�������,������ӽڵ�,���ȴ����￪ʼ
//uiMemPosUsing:  ʵ��ʹ�õĵ�ַ,��ǰ�ڵ�ŵ��ڴ�ʱ�ĵ�ַ
//uiMemUseTotal:  �ܹ����˶�����,������,��ʹ�þʹ����￪ʼ
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
    xhRbNodeNew.usWeight = usWMid > usWMy ? usWMid : usWMy; //����ҫ��ϵ��
	clsTreeNode->usWeight = xhRbNodeNew.usWeight; //�޸�ԭʼ���нڵ��Ȩ��

#if _WITH_DEBUG_ //��ӡ������,����
    {
        char szLog[1024];
        snprintf(szLog, sizeof(szLog), "--------->>node");
		CWriteLog::GetInstance().WriteLog(szLog);
        snprintf(szLog, sizeof(szLog), "         >>origsize:%lu| composesize:%u", sizeof(xhRbNodeNew), xhRbNodeNew.GetComposeSize());  CWriteLog::GetInstance().WriteLog(szLog);
        snprintf(szLog, sizeof(szLog), "         >>UseMemory:%u", uiMemUseTotal); CWriteLog::GetInstance().WriteLog(szLog);
        snprintf(szLog, sizeof(szLog), "         >>left:%u; right:%u; usWMy:%u; usWeight:%u", usWLeft, usWRight, usWMy, clsTreeNode->usWeight); CWriteLog::GetInstance().WriteLog(szLog);
    }
#endif

    xhRbNodeNew.ComposeCut(); //ѹ��
    int iComposeSize = xhRbNodeNew.GetComposeSize();

    if(clsTreeRoot == clsTreeNode) { //�Ǹ��ڵ��ʱ��,���ٶ�����ռ�
        lseek(iFileFd, SEEK_SET, 0);
        write(iFileFd, &xhRbNodeNew, 4); //������4���ֽ�
        write(iFileFd, &xhRbNodeNew, iComposeSize);
    } else {
        write(iFileFd, &xhRbNodeNew, iComposeSize);
        uiMemPosUsing = uiMemUseTotal; //��ǰ�ڵ���׵�ַ
        uiMemUseTotal += iComposeSize;
    }
    return bRet;
}
//0->������
//1->��ƴ
//2->ȫƴ
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
        write(iFdOut, &uiUseTotal, 4); //д4���ֽ�,������'��չ��Ϣ�ĳ���'����

		if(!clsTreeNode) break;
        //---------------->>>�Ѹ��ڵ�ŵ���һ��λ��==========begin
        int iTreeRootSize = (clsTreeNode->pleft_ ? 1 : 0) + (clsTreeNode->pright_ ? 1 : 0) + (clsTreeNode->pmiddle_ ? 1 : 0) + (clsTreeNode->pitem_ ? 1 : 0);
        iTreeRootSize = sizeof(XHRbNode) - sizeof(unsigned int) * ( 4 - iTreeRootSize);
        write(iFdOut, szLog, iTreeRootSize);
        uiMemBegin += iTreeRootSize;
        uiUseTotal += iTreeRootSize;
        //========================================end
        //���simpleself��ǰ�ĸ��ֽ�->first
        unsigned int uiSSLen = 4;
		if(iFdMS != -1) {
			write(iFdMS, &uiSSLen, 4);
		}
        //���simpleself��ǰ�ĸ��ֽ�--->over

        Do_WriteTreeNode(iFdMS, csToSpell, clsTreeNode, uiMemBegin, uiNodeBegin, uiUseTotal, iFdOut, clsTreeNode, uiSSLen, NULL, iMixSearch);

        //���simpleself��ǰ�ĸ��ֽ�->second
		if(iFdMS != -1) {
			lseek(iFdMS, SEEK_SET, 0);
			write(iFdMS, &uiSSLen, 4);
		}
        //���simpleself��ǰ�ĸ��ֽ�--->over

        //printf("================%d, begin:%d, total:%d\n", iTreeRootSize, uiMemBegin, uiUseTotal);
        lseek(iFdOut, SEEK_SET, 0);
        write(iFdOut, &uiUseTotal, 4); //old û�аѸ��ڵ���ǰʱ
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
		//ֻ��ƴ��������peer��Ϣ
		if(iSelectId == SELECT_TREE_SPELL || iSelectId == SELECT_TREE_CENTER_SPELL || iSelectId == SELECT_TREE_EX_SPELL || iSelectId == SELECT_TREE_EX_CENTER_SPELL) {
			bRet = MarkHavePeerInfo(pTreeNodeRoot);
			if(bRet == false) break;
		}
		bRet = WriteTreeNode(iSelectId, pTreeNodeRoot, csToSpell);
		if(bRet == false) break;
	}while(0);

	//�ͷſռ�
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
