#include "isindex/IndexDataInfo.h"
#include "util/FileCommon.h"
#include "util/ReadBigFile.h"
#include "util/XHStrUtils.h"
#include "isindex/CreateIndex.h"

//创建一些除了树之外的信息索引,以及临时信息
//为创建树做准备


int CIndexDataInfo::Run(const string & strTaskName, CSysConfigSet *pclsSysConfigSet)
{
	m_strTaskName = strTaskName;
	m_pSysConfigSet = pclsSysConfigSet;
	InitParamter();
	Process();
}

void CIndexDataInfo::InitParamter()
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
	m_bDictNeedCreate[SELECT_TREE_CHINESE] = 1; //必须要处理汉字字典
}

//处理基础信息（公共要使用到的一些信息）
int CIndexDataInfo::Process()
{
	Process_BasicInfo_Chinese();
	Process_BasicInfo_Spell();
	Process_BasicInfo_Simple();
	return 0;
}

int CIndexDataInfo::SelectChangeFromEx(int iSelect)
{
	switch(iSelect) {
		case SELECT_TREE_EX_CHINESE       : return SELECT_TREE_CHINESE       ;
		case SELECT_TREE_EX_SPELL         : return SELECT_TREE_SPELL         ;
		case SELECT_TREE_EX_SIMPLE        : return SELECT_TREE_SIMPLE        ;
		case SELECT_TREE_EX_CENTER_CHINESE: return SELECT_TREE_CENTER_CHINESE;
		case SELECT_TREE_EX_CENTER_SPELL  : return SELECT_TREE_CENTER_SPELL  ;
		case SELECT_TREE_EX_CENTER_SIMPLE : return SELECT_TREE_CENTER_SIMPLE ;
	}
	return iSelect;
}

int CIndexDataInfo::Deal_WordStringInfo(CWordStringInfo & clsWSI, int iType)
{
	//调用CWordStringInfo的有下面三种：
	//第一种是汉字树，且必须为汉字树
	//第二种为拼音树，选择顺序为：扩展拼音，拼音，中间扩展拼音，中间拼音。都没有则忽略
	//第三种简拼树，选择顺序为：扩展简拼，简拼，中间扩展简拼，中间简拼。都没有则忽略

	string strPath = m_strDataPath + "/" + m_strTaskName + "/";
	//汉字字符串 & 汉字扩展
	if(iType == 0) {
		string strChinese = strPath + m_pSysConfigSet->GetFileName(SELECT_TREE_CHINESE);
		string strChineseString = strPath + m_pSysConfigSet->GetString(PARA_CONFIG_chinesestring);
		string strChineseExtra = strChinese + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_extra);
		clsWSI.AnlyInfo(strChinese, strChineseString, strChineseExtra, EXTRA_INFO_SET_INSIDE);

	} else if(iType == 1) {
		//spell
		int iSelectSpellId = -1;
		if(m_bDictNeedCreate[SELECT_TREE_EX_SPELL]) {
			iSelectSpellId = SELECT_TREE_EX_SPELL;
		} else if(m_bDictNeedCreate[SELECT_TREE_SPELL]) {
			iSelectSpellId = SELECT_TREE_SPELL;
		} else if(m_bDictNeedCreate[SELECT_TREE_EX_CENTER_SPELL]) {
			iSelectSpellId = SELECT_TREE_EX_CENTER_SPELL;
		} else if(m_bDictNeedCreate[SELECT_TREE_CENTER_SPELL]) {
			iSelectSpellId = SELECT_TREE_CENTER_SPELL;
		}
		if(iSelectSpellId != -1) {
			string strFile = strPath + m_pSysConfigSet->GetFileName(iSelectSpellId);
			string strSpellString = strPath + m_pSysConfigSet->GetString(PARA_CONFIG_spellstring);
			clsWSI.AnlyInfo(strFile, strSpellString);
		}
	} else if(iType == 2) {
		//simple
		int iSelectSimpleId = -1;
		if(m_bDictNeedCreate[SELECT_TREE_EX_SIMPLE]) {
			iSelectSimpleId = SELECT_TREE_EX_SIMPLE;
		} else if(m_bDictNeedCreate[SELECT_TREE_SIMPLE]) {
			iSelectSimpleId = SELECT_TREE_SIMPLE;
		} else if(m_bDictNeedCreate[SELECT_TREE_EX_CENTER_SIMPLE]) {
			iSelectSimpleId = SELECT_TREE_EX_CENTER_SIMPLE;
		} else if(m_bDictNeedCreate[SELECT_TREE_CENTER_SIMPLE]) {
			iSelectSimpleId = SELECT_TREE_CENTER_SIMPLE;
		}
		if(iSelectSimpleId != -1) {
			string strFile = strPath + m_pSysConfigSet->GetFileName(iSelectSimpleId);
			string strSimpleString = strPath + m_pSysConfigSet->GetString(PARA_CONFIG_simplestring);
			clsWSI.AnlyInfo(strFile, strSimpleString);
		}
	}
	return 0;

}

//处理汉字词典
void CIndexDataInfo::CreateOneIndex_Chinese1(CWordStringInfo & clsWSI, const int iSelectId, const int & iNodeNum)
{
	XHChineseData * pNodeData = new XHChineseData[iNodeNum];
	if(pNodeData == NULL) {
		printf("error: new memory failed[%d]\n", iNodeNum);
		return ;
	}
	memset(pNodeData, 0x00, sizeof(XHChineseData) * iNodeNum);

	string strPath = m_strDataPath + "/" + m_strTaskName + "/";
	string strFileName = strPath + m_pSysConfigSet->GetFileName(iSelectId);
	CReadBigFile fdR;
	fdR.Open(strFileName.c_str());
	char *p;
	int iFileLines = -1;
	char szBody[10240];
	int iLoop;
	while(fdR.GetLine(p)) {
		iFileLines++;
		XHChineseData * pD = pNodeData + iFileLines;
		//查找是否有|,扩展信息
		for(iLoop = 0; p[iLoop] != '|' && p[iLoop] != 0; iLoop++) {
			szBody[iLoop] = p[iLoop];
			if(p[iLoop] & 0x80) {
				iLoop++;
				szBody[iLoop] = p[iLoop];
			}
		}
		if(p[iLoop] == '|') { //含有扩展信息
			pD->ucAttr |= FLAG_CHINESE_UCATTR_WITHEXTRAINFO;
			szBody[iLoop] = 0;
		}
		vector<string> vecSplit;
		pD->usAttr = 0;
		XHStrUtils::StrTokenizeGBK(vecSplit, szBody, ",");
		if(vecSplit.size() < 5) {
			if(vecSplit.size() < 4) {
				printf("paramter is less than 4[%d](%s)(%s):%d(%s)[%s %d]\n", vecSplit.size(), p, szBody, iLoop, strFileName.c_str(), __FILE__, __LINE__);
				break;
			}
		} else {
			pD->usAttr = (unsigned short)atoi(vecSplit[4].c_str());
		}
		string strKey = vecSplit[0];
		string strPre = vecSplit[1];
		if(strKey != vecSplit[2]) { //格式化后的词是否与原词相同
			pD->ucAttr |= FLAG_CHINESE_UCATTR_WITHORIGWORD;
		}
		int iWeight = atoi(vecSplit[3].c_str());
		if(iWeight > MAX_WEIGHT_CHINESE) {
			iWeight = MAX_WEIGHT_CHINESE;
		} else if(iWeight < 0) {
			iWeight = 0;
		}
		pD->usWeight = iWeight;
		unsigned int uiKeyPos = clsWSI.GetWordPos(strPre + "-" + strKey);
		if(uiKeyPos == 0) {
			printf("error:(exit) not found the key:{%s}:{%s}{%s}[%s %d]\n", p, strKey.c_str(), szBody, __FILE__, __LINE__);
			exit(-1);
		}
		pD->uiStringPos = uiKeyPos;
	}

	//让iFileLines为行数
	iFileLines++;
	//输出到文件
	string strFileNameNodeData = strFileName + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_nodedata);
	printf("-------------write-->%s[file line:%d]\n", strFileNameNodeData.c_str(), iFileLines);
	CFileCommon::WriteOneFile(strFileNameNodeData, (const char *)pNodeData, sizeof(XHChineseData) * iNodeNum);

	delete []pNodeData;
}
//nodedata同ex采用了压缩的方式进行存储,压缩的原理相似,以ex_为例解释
//ex_汉字
//    汉字extra文件
//先: 总共占有的大小  汉字extra的实体部分
//后: 总大小(包含ex_) 汉字extra的实体部分 ex_汉字extra的实体部分
//ex_其它
//    其它extra文件
//先: 总共占有的大小  其它extra的实体部分
//后: 总大小(包含ex_) 其它extra的实体部分 ex_其它extra的实体部分(如果对应的非ex_为1个结果,那么这一个结果利用前面"其它extra的实体部分",多个结果,则写在后面)
void CIndexDataInfo::CreateOneIndex_Chinese2(CWordStringInfo & clsWSI, const int iSelectId, const int & iNodeNum)
{
	XHSpellData * pNodeData = new XHSpellData[iNodeNum];
	if(pNodeData == NULL) {
		printf("error: new memory failed[%d]\n", iNodeNum);
		return ;
	}
	memset(pNodeData, 0x00, sizeof(XHSpellData) * iNodeNum);

	//扩展信息
	int iNewSelectId = SelectChangeFromEx(iSelectId);
	unsigned int uiExtraSize = 4;
	string strPath = m_strDataPath + "/" + m_strTaskName + "/";
	string strFileNameOrig = strPath + m_pSysConfigSet->GetFileName(iSelectId);
	string strFileName = strPath + m_pSysConfigSet->GetFileName(iNewSelectId);
	string strFileExtra = strFileName + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_extra);
	int iFdExtra = 0;
	if(!m_bDictNeedCreate[iNewSelectId] || iNewSelectId == iSelectId) {
		iFdExtra = open(strFileExtra.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		write(iFdExtra, &uiExtraSize, 4);
	} else {
		iFdExtra = open(strFileExtra.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
		lseek(iFdExtra, 0, SEEK_SET);
		read(iFdExtra, &uiExtraSize, 4);
		lseek(iFdExtra, 0, SEEK_END);
	}

	//合并扩展使用
	IdToId * pIdToId = NULL;
	bool bJoin = false;
	int iAppendLines = 0;
	map<const string, unsigned int> mapWordLineId;
	map<const string, unsigned int>::iterator iIterMI;
	if(iSelectId != iNewSelectId) { //当前是扩展处理
		pIdToId = new IdToId[iNodeNum];
		if(pIdToId == NULL) {
			printf("error:(exit) new space error(%d)[%s %d]\n", iNodeNum, __FILE__, __LINE__);
			exit(-1);
		}
		//如果存在中间汉字,在处理扩展的中间汉字时,使用压缩的机制.把扩展的中间汉字和中间汉字放到一起
		bJoin = true;
		string strFileT = strPath + m_pSysConfigSet->GetFileName(iNewSelectId);
		if(SELECT_TREE_EX_CHINESE != iSelectId) { //扩展的汉字不进行压缩,只是简单的合并
			CFileCommon::WordToLineNumber(strFileT, ",", 1, mapWordLineId);
		}
		iAppendLines = CFileCommon::GetFileLines(strFileT);
	}

	CReadBigFile fdR;
	fdR.Open(strFileNameOrig.c_str());
	char *p;
	int iFileLines = -1;
	int iLoop;
	int iFileLinesAppend = -1;
	while(fdR.GetLine(p)) {
		iFileLinesAppend++;
		vector<string> vecSplit;
		XHStrUtils::StrTokenizeGBK(vecSplit, p, "|");
		if(vecSplit.size() < 2) {
			printf("error:(exit)%s not 2 paramter(%d)[%s %d]\n", p, vecSplit.size(), __FILE__, __LINE__);
			exit(-1);
		}
		vector<string> vecSplitBody, vecSplitExtra;
		XHStrUtils::StrTokenizeGBK(vecSplitBody, vecSplit[0], ",");
		XHStrUtils::StrTokenizeGBK(vecSplitExtra, vecSplit[1], ",");
		if(vecSplitBody.size() != 3) {
			printf("error:(exit)paramter is less than 3[%d](%s)[%s:%d]\n", vecSplitBody.size(), p, __FILE__, __LINE__);
			exit(-1);
		}
		string strKey = vecSplitBody[1];
		if(bJoin) {
			//扩展的汉字不进行压缩,只是简单的合并
			if(SELECT_TREE_EX_CHINESE != iSelectId && vecSplitExtra.size() == 1 && (iIterMI = mapWordLineId.find(strKey)) != mapWordLineId.end()) { //找到(在只有个上级id的情况下)
				pIdToId[iFileLinesAppend].uiTo = iIterMI->second;
				continue;
			} else {
				pIdToId[iFileLinesAppend].uiTo = iFileLines + 1 + iAppendLines;
			}
		}
		unsigned int uiKeyPos;
		//中间拼音/简拼 特殊处理
		if(iSelectId == SELECT_TREE_CENTER_SPELL || iSelectId == SELECT_TREE_EX_CENTER_SPELL || iSelectId == SELECT_TREE_CENTER_SIMPLE || iSelectId == SELECT_TREE_EX_CENTER_SIMPLE) {
			uiKeyPos = clsWSI.GetReverseWordPos(strKey);
		} else {
			uiKeyPos = clsWSI.GetWordPos(strKey);
		}
		if(uiKeyPos == 0) {
			printf("error:(exit) not found the key:{%s}:{%s}{tree:%d}[%s %d]\n", p, strKey.c_str(), iSelectId, __FILE__, __LINE__);
			exit(-1);
		}
		iFileLines++;
		XHSpellData* pD = pNodeData + iFileLines;
		if(vecSplitExtra.size() > 1) {
			pD->bMD = 1;
			int iSize = vecSplitExtra.size();
			write(iFdExtra, &iSize, 4);
			uiExtraSize += 4;
			pD->uiUpperPos = uiExtraSize;
			for(iLoop = 0; iLoop < iSize; iLoop++) {
				unsigned int uiNum = (unsigned int)atoi(vecSplitExtra[iLoop].c_str()); //因为不能超过2G，因此这里的整形不能为负
				write(iFdExtra, &uiNum, 4);
				uiExtraSize += 4;
			}
		} else if(vecSplitExtra.size() == 1) {
			pD->bMD = 0;
			pD->uiUpperPos = (unsigned int)atoi(vecSplitExtra[0].c_str());
		} else {
			printf("error:(exit) extra size is zero:%s[%s %d]\n", p, __FILE__, __LINE__);
			exit(-1);
		}
		pD->uiStringPos = uiKeyPos;
		
	}
	lseek(iFdExtra, 0, SEEK_SET);
	write(iFdExtra, &uiExtraSize, 4);
	close(iFdExtra);

	//输出到文件
	string strFileNameNodeData = strFileName + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_nodedata);
	printf("-------------write-->%s[file line:%d]\n", strFileNameOrig.c_str(), iFileLines + 1);
	if(bJoin) {
		string strFileIdToId = strFileNameOrig + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_nodedata);
		CFileCommon::WriteOneFile(strFileIdToId, (const char *)pIdToId, sizeof(IdToId) * iNodeNum);
		if(iFileLines != -1) {
			//让iFileLines为行数
			iFileLines++;
			if(SELECT_TREE_EX_CHINESE == iSelectId || m_bDictNeedCreate[iNewSelectId]) {
				CFileCommon::WriteOneFileAppend(strFileNameNodeData, (const char *)pNodeData, sizeof(XHSpellData) * iFileLines);
			} else { //如果只有扩展,没有非扩展
				CFileCommon::WriteOneFile(strFileNameNodeData, (const char *)pNodeData, sizeof(XHSpellData) * iFileLines);
			}
		}
		mapWordLineId.erase(mapWordLineId.begin(), mapWordLineId.end());
	} else {
		if(iFileLines != -1) {
			//让iFileLines为行数
			iFileLines++;
			CFileCommon::WriteOneFile(strFileNameNodeData, (const char *)pNodeData, sizeof(XHSpellData) * iFileLines);
		}
	}

	SAFE_DELETE_MUL(pNodeData);
	SAFE_DELETE_MUL(pIdToId);
}
//中间汉字，扩展中间汉字
//这里获取字符串位置的方式：
//加载汉字nodedata，然后根据“中间汉字|扩展中间汉字”的扩展信息得到一个汉字nodedata，
//然后再根据该nodedata的uiStringPos，得到一个字符串。最后找出偏移，计算出最终结果。
void CIndexDataInfo::CreateOneIndex_Chinese3(const int iSelectId, const int & iNodeNum)
{
	string strPath = m_strDataPath + "/" + m_strTaskName + "/";
	XHSpellData * pNodeData = new XHSpellData[iNodeNum];
	if(pNodeData == NULL) {
		printf("error: new memory failed[%d]\n", iNodeNum);
		return ;
	}
	memset(pNodeData, 0x00, sizeof(XHSpellData) * iNodeNum);

	//读取汉字的nodedata
	string strChineseFileName = strPath + m_pSysConfigSet->GetFileName(SELECT_TREE_CHINESE);
	string strChineseNodeData = strChineseFileName + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_nodedata);
	int iFdChineseNodeData;
	unsigned int uiChineseNodeDataSize = 0;
	unsigned int uiChineseNodeDataNum = 0;
	iFdChineseNodeData = open(strChineseNodeData.c_str(), O_RDONLY);
	read(iFdChineseNodeData, &uiChineseNodeDataSize, 4);
	uiChineseNodeDataNum = (uiChineseNodeDataSize - 4)/ sizeof(XHChineseData);
	XHChineseData * pChineseNodeData = new XHChineseData[uiChineseNodeDataNum];
	if(pChineseNodeData == NULL) {
		printf("error:(exit) new space error:%u[%s %d]\n", uiChineseNodeDataSize, __FILE__, __LINE__);
		exit(-1);
	}
	//这是曾经的写法
	//read(iFdChineseNodeData, (char *)pChineseNodeData, uiChineseNodeDataSize);
	//上面的写法导致了我用了半天的时间调一个core的bug,说我对于一个内存重复释放,为了纪念那半天的时间,特别注释
	read(iFdChineseNodeData, (char *)pChineseNodeData, uiChineseNodeDataSize - 4);
	close(iFdChineseNodeData);
	//汉字字符串信息
	string strChineseString = strPath + m_pSysConfigSet->GetString(PARA_CONFIG_chinesestring);
	int iFdChineseString;
	unsigned int uiChineseStringSize = 0;
	iFdChineseString = open(strChineseString.c_str(), O_RDONLY);
	read(iFdChineseString, &uiChineseStringSize, 4);
	//uiChineseStringSize = uiChineseStringSize - 4;
	char * pChineseString = new char[uiChineseStringSize];
	if(pChineseString == NULL) {
		printf("error:(exit) new space error:%u[%s %d]\n", uiChineseStringSize, __FILE__, __LINE__);
		exit(-1);
	}
	read(iFdChineseString, pChineseString + 4, uiChineseStringSize);
	close(iFdChineseString);

	//扩展信息
	unsigned int uiExtraSize = 4;
	int iNewSelectId = SelectChangeFromEx(iSelectId);
	string strFileName = strPath + m_pSysConfigSet->GetFileName(iNewSelectId);
	string strFileNameOrig = strPath + m_pSysConfigSet->GetFileName(iSelectId);
	string strFileExtra = strFileName + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_extra);
	int iFdExtra; 
	//如果不存在非扩展,则只有扩展,创建文件的时候就清空之前的文件里面的内容
	if(!m_bDictNeedCreate[iNewSelectId] || iNewSelectId == iSelectId) {
		iFdExtra = open(strFileExtra.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		write(iFdExtra, &uiExtraSize, 4);
	} else {
		//iFdExtra = open(strFileExtra.c_str(), O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);
		iFdExtra = open(strFileExtra.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
		lseek(iFdExtra, 0, SEEK_SET);
		read(iFdExtra, &uiExtraSize, 4);
		lseek(iFdExtra, 0, SEEK_END);
	}

	//合并扩展使用
	IdToId * pIdToId = NULL;
	bool bJoin = false;
	map<const string, unsigned int> mapWordLineId;
	map<const string, unsigned int>::iterator iIterMI;
	if(iSelectId != iNewSelectId) { //当前是扩展处理
		pIdToId = new IdToId[iNodeNum];
		if(pIdToId == NULL) {
			printf("error:(exit) new space error(%d)[%s %d]\n", iNodeNum, __FILE__, __LINE__);
			exit(-1);
		}
		//如果存在中间汉字,在处理扩展的中间汉字时,使用压缩的机制.把扩展的中间汉字和中间汉字放到一起
		bJoin = true;
		string strCenterChinese = strPath + m_pSysConfigSet->GetFileName(iNewSelectId);
		CFileCommon::WordToLineNumber(strCenterChinese, ",", 1, mapWordLineId);
	}

	CReadBigFile fdR;
	fdR.Open(strFileNameOrig.c_str());
	char *p;
	int iFileLines = -1;
	int iLoop;
	int iFileLinesAppend = -1;
	while(fdR.GetLine(p)) {
		iFileLinesAppend++;
		vector<string> vecSplit;
		XHStrUtils::StrTokenizeGBK(vecSplit, p, "|");
		if(vecSplit.size() < 2) {
			printf("error:(exit)%s[%s] not 2 paramter(%d)[%s %d]\n", p, strFileNameOrig.c_str(), vecSplit.size(), __FILE__, __LINE__);
			exit(-1);
		}
		vector<string> vecSplitBody, vecSplitExtra;
		XHStrUtils::StrTokenizeGBK(vecSplitBody, vecSplit[0], ",");
		XHStrUtils::StrTokenizeGBK(vecSplitExtra, vecSplit[1], ",");
		if(vecSplitBody.size() != 3) {
			printf("error:(exit)paramter is less than 3[%d](%s)[%s:%d]\n", vecSplitBody.size(), p, __FILE__, __LINE__);
			exit(-1);
		}
		string strKey = vecSplitBody[1];
		if(bJoin) {
			if(vecSplitExtra.size() == 1 && (iIterMI = mapWordLineId.find(strKey)) != mapWordLineId.end()) { //找到(在只有个上级id的情况下)
				pIdToId[iFileLinesAppend].uiTo = iIterMI->second;
				continue;
			} else {
				pIdToId[iFileLinesAppend].uiTo = iFileLinesAppend;
			}
		}
		iFileLines++;
		XHSpellData* pD = pNodeData + iFileLines;
		if(vecSplitExtra.size() > 1) {
			pD->bMD = 1;
			int iSize = vecSplitExtra.size();
			write(iFdExtra, &iSize, 4);
			uiExtraSize += 4;
			pD->uiUpperPos = uiExtraSize;
			for(iLoop = 0; iLoop < iSize; iLoop++) {
				unsigned int uiNum = (unsigned int)atoi(vecSplitExtra[iLoop].c_str()); //因为不能超过2G，因此这里的整形不能为负
				write(iFdExtra, &uiNum, 4);
				uiExtraSize += 4;
			}
		} else if(vecSplitExtra.size() == 1) {
			pD->bMD = 0;
			pD->uiUpperPos = (unsigned int)atoi(vecSplitExtra[0].c_str());
		} else {
			printf("error:(exit) extra size is zero:%s[%s %d]\n", p, __FILE__, __LINE__);
			exit(-1);
		}
		//重新计算字符串的位置
		unsigned int uiChineseSubId = (unsigned int)atoi(vecSplitExtra[0].c_str());
		unsigned int uiStringPos = pChineseNodeData[uiChineseSubId].uiStringPos; // - 4;
		string strValue = pChineseString + uiStringPos;
		unsigned int uiKeyPos = uiStringPos + strValue.size() - strKey.size();
		pD->uiStringPos = uiKeyPos;
		
	}
	lseek(iFdExtra, 0, SEEK_SET);
	write(iFdExtra, &uiExtraSize, 4);
	close(iFdExtra);

	//输出到文件
	string strFileNameNodeData = strFileName + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_nodedata);
	printf("-------------write-->%s[file line:%d]\n", strFileNameOrig.c_str(), iFileLines + 1);

	if(bJoin) {
		string strFileIdToId = strFileNameOrig + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_nodedata);
		CFileCommon::WriteOneFile(strFileIdToId, (const char *)pIdToId, sizeof(IdToId) * iNodeNum);
		if(iFileLines != -1) {
			//让iFileLines为行数
			iFileLines++;
			if(m_bDictNeedCreate[iNewSelectId]) { //如果只有扩展,没有非扩展
				CFileCommon::WriteOneFileAppend(strFileNameNodeData, (const char *)pNodeData, sizeof(XHSpellData) * iFileLines);
			} else {
				CFileCommon::WriteOneFile(strFileNameNodeData, (const char *)pNodeData, sizeof(XHSpellData) * iFileLines);
			}
		}
		mapWordLineId.erase(mapWordLineId.begin(), mapWordLineId.end());
	} else {
		if(iFileLines != -1) {
			//让iFileLines为行数
			iFileLines++;
			CFileCommon::WriteOneFile(strFileNameNodeData, (const char *)pNodeData, sizeof(XHSpellData) * iFileLines);
		}
	}
	SAFE_DELETE_MUL(pChineseString);
	SAFE_DELETE_MUL(pNodeData);
	SAFE_DELETE_MUL(pChineseNodeData);
	SAFE_DELETE_MUL(pIdToId);
}
void CIndexDataInfo::CreateOneIndex(CWordStringInfo & clsWSI, const int iSelectId)
{
	if(!m_bDictNeedCreate[iSelectId] && iSelectId != SELECT_TREE_CHINESE) return; //不需要建立
	string strPath = m_strDataPath + "/" + m_strTaskName + "/";
	string strFileName = strPath + m_pSysConfigSet->GetFileName(iSelectId);
	int iNodeDataNum = CFileCommon::GetFileLines(strFileName);
	if(iNodeDataNum == 0) {
		printf("file line number is zero(%s)\n", strFileName.c_str());
		return;
	}

	switch(iSelectId) {
		case SELECT_TREE_CHINESE:
			CreateOneIndex_Chinese1(clsWSI, iSelectId, iNodeDataNum);
			break;
		case SELECT_TREE_CENTER_CHINESE: case SELECT_TREE_EX_CENTER_CHINESE: //不使用clsWSI
			CreateOneIndex_Chinese3(iSelectId, iNodeDataNum);
			break;
		case SELECT_TREE_EX_CHINESE:
		case SELECT_TREE_SPELL:
		case SELECT_TREE_SIMPLE:
		case SELECT_TREE_CENTER_SPELL:
		case SELECT_TREE_CENTER_SIMPLE:
			//特殊扩展
		case SELECT_TREE_EX_SPELL:  case SELECT_TREE_EX_SIMPLE:  
		case SELECT_TREE_EX_CENTER_SPELL: case SELECT_TREE_EX_CENTER_SIMPLE:
			CreateOneIndex_Chinese2(clsWSI, iSelectId, iNodeDataNum);
			break;
		default:
			break;
	}
}

//注意 如果同时存在扩展和非扩展,应该首先处理非扩展,汉字/全拼/简拼相同
//并且必须成对执行
void CIndexDataInfo::Process_BasicInfo_Chinese()
{
	CWordStringInfo clsWSI;
	Deal_WordStringInfo(clsWSI, 0);

	CreateOneIndex(clsWSI, SELECT_TREE_CHINESE);
	CreateOneIndex(clsWSI, SELECT_TREE_EX_CHINESE);
	clsWSI.Release();
	CreateOneIndex(clsWSI, SELECT_TREE_CENTER_CHINESE);
	CreateOneIndex(clsWSI, SELECT_TREE_EX_CENTER_CHINESE);
}
void CIndexDataInfo::Process_BasicInfo_Spell()
{
	CWordStringInfo clsWSI;
	Deal_WordStringInfo(clsWSI, 1);
	CreateOneIndex(clsWSI, SELECT_TREE_SPELL);
	CreateOneIndex(clsWSI, SELECT_TREE_EX_SPELL);
	if(m_bDictNeedCreate[SELECT_TREE_CENTER_SPELL] || m_bDictNeedCreate[SELECT_TREE_EX_CENTER_SPELL]) {
		clsWSI.Reverse();
		clsWSI.Release();
		CreateOneIndex(clsWSI, SELECT_TREE_CENTER_SPELL);
		CreateOneIndex(clsWSI, SELECT_TREE_EX_CENTER_SPELL);
	}
}
void CIndexDataInfo::Process_BasicInfo_Simple()
{
	CWordStringInfo clsWSI;
	Deal_WordStringInfo(clsWSI, 2);
	CreateOneIndex(clsWSI, SELECT_TREE_SIMPLE);
	CreateOneIndex(clsWSI, SELECT_TREE_EX_SIMPLE);
	if(m_bDictNeedCreate[SELECT_TREE_CENTER_SIMPLE] || m_bDictNeedCreate[SELECT_TREE_EX_CENTER_SIMPLE]) {
		clsWSI.Reverse();
		clsWSI.Release();
		CreateOneIndex(clsWSI, SELECT_TREE_CENTER_SIMPLE);
		CreateOneIndex(clsWSI, SELECT_TREE_EX_CENTER_SIMPLE);
	}
}
