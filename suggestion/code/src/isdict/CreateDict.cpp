#include "util/Pinyin.h"
#include "isdict/CreateDict.h"
#include "util/ConfigReader.h"
#include "util/ReadBigFile.h"
#include "util/XHStrUtils.h"
#include "segment/WordSplitter.h"

int CCreateDict::Run(const string & strTaskName, CSysConfigSet *pclsSysConfigSet)
{
	m_strTaskName = strTaskName;
	m_pSysConfigSet = pclsSysConfigSet;
	m_strTmpSuffix  = ".dict";
	Init();
	Process();
	return 0;
}


int CCreateDict::WriteOneDict(const string & strPre, const string & strKey, CWriteBigFile & wFD, const int iWeight, const int iCount, const string & strExtra2)
{
	char szInf[256];
	snprintf(szInf, sizeof(szInf) / sizeof(szInf[0]), ",%d|%d", iWeight, iCount);
	string strWrite = strPre + "," + strKey + szInf;
	if(strExtra2 != "") {
		strWrite += "|" + strExtra2;
	}

	wFD.WriteLine(strWrite.c_str());
	return 0;
}

void CCreateDict::Init()
{
	InitParamter();
	InitLoadSplitProc();
}
void CCreateDict::InitParamter()
{
	m_strDataPath              = m_pSysConfigSet->GetString(PARA_CONFIG_pathdata);
	m_iMaxSplitNum             = m_pSysConfigSet->GetInt(PARA_CONFIG_centerchineseMostnumber);
	m_iMinSplitCenterWordLen   = m_pSysConfigSet->GetInt(PARA_CONFIG_mincentersearchwordlen);
	m_iMinSplitCenterSearchLen = m_pSysConfigSet->GetInt(PARA_CONFIG_mincentersearchlen);
	m_iMaxSplitCenterSearchLen = m_pSysConfigSet->GetInt(PARA_CONFIG_maxcentersearchlen);
	//初始化要生成的dict
	string strDictNeed = m_pSysConfigSet->GetString(PARA_CONFIG_selecttree);
	vector<string> vecDictNeed;
	XHStrUtils::StrTokenize(vecDictNeed, strDictNeed, ";");
	int iLoop;
	//executed by id asc
	memset(m_bDictNeedCreate, 0x00, sizeof(m_bDictNeedCreate));
	m_bNeedPinyinDeal = false;
	for(iLoop = 0; iLoop < vecDictNeed.size(); iLoop++) {
		int iSub = atoi(vecDictNeed[iLoop].c_str());
		if(iSub >= 0 && iSub <= sizeof(m_bDictNeedCreate) / sizeof(m_bDictNeedCreate[0])) {
			m_bDictNeedCreate[iSub] = 1;
			if(WITH_PINYIN_TREE(iSub)) {
				m_bNeedPinyinDeal = true;
			}
		}
	}
	m_bDictNeedCreate[SELECT_TREE_CHINESE] = 0; //该文件不处理汉字词典
}

//加载分词程序，只加载一次
void CCreateDict::InitLoadSplitProc()
{
	static bool bFirst = true;
	if(!bFirst) {
		return ;
	}
	//m_pSysConfigSet->Debug_Print();
	bFirst = false;
	string strCfgPath = m_pSysConfigSet->GetString(PARA_CONFIG_configurepath);
	string strSplitWordPath = m_pSysConfigSet->GetString(PARA_CONFIG_splitwordpath);
	strSplitWordPath = strCfgPath + "/" + strSplitWordPath;
	WordSplitter::initial(strSplitWordPath);

	//初始化拼音
	string strPinyin = strCfgPath + "/pinyin";
	string strMPinyin = strCfgPath + "/mpinyin";
	Pinyin::getInstance(strPinyin, strMPinyin);
}

//包含空格
void CCreateDict::SplitWord(const string & strIn, vector<string> &vecOut)
{
	int iSizeC = strIn.size();
	//判断是否长度在限制的范围之内
	if(iSizeC > m_iMaxSplitCenterSearchLen || iSizeC < m_iMinSplitCenterSearchLen) {
		vecOut.push_back(strIn);
		return ;
	}
	WordSplitsResult wsrSplitWordSplit;
	WordSplitter::getInstance().split(strIn, wsrSplitWordSplit);
	int iSize = wsrSplitWordSplit.mixedWords.size();
	int iLoop;
	for(iLoop = 0; iLoop < iSize; iLoop++) {
		vecOut.push_back(wsrSplitWordSplit.mixedWords[iLoop].word);
	}
}
//SplitWordWithPinyin已经是有点乱了，这个就单独写出来吧
//这个函数没有什么重大作用，只是给只要输出汉字树时加速
void CCreateDict::SplitWordNoPinyin(const string & strIn, vector<WordToSpell> & vecPinyin)
{
	vector<string> vecSplit;
	SplitWord(strIn, vecSplit);
	vector<WordToSpell> vecPinyinTmp;
	int iLoop;
	WordToSpell stWTSLast;
	for(iLoop = vecSplit.size() - 1; iLoop >= 0; iLoop--) {
		WordToSpell stWTS;
		string strWord = vecSplit[iLoop];
		stWTS.strWord = strWord;
		if(strWord == " ") {
			stWTSLast.strWord = strWord + stWTSLast.strWord;
			continue;
		}

		vecPinyinTmp.push_back(stWTS);
		stWTSLast = stWTS;
	}
	//反置
	for(iLoop = vecPinyinTmp.size() - 1; iLoop >= 0; iLoop--) {
		vecPinyin.push_back(vecPinyinTmp[iLoop]);
	}
}

//分词然后获取全拼/简拼/简拼对应的全拼
void CCreateDict::SplitWordWithPinyin(const string & strIn, vector<WordToSpell> & vecPinyin)
{
	vector<string> vecSplit;
	SplitWord(strIn, vecSplit);
	int iLoop, jLoop;
	int iSize;
	WordToSpell stWTSLast;
	vector<WordToSpell> vecPinyinTmp;
	for(iLoop = vecSplit.size() - 1; iLoop >= 0; iLoop--) {
		WordToSpell stWTS;
		string strWord = vecSplit[iLoop];
		stWTS.strWord = strWord;
		if(strWord == " ") { //如果输入的如:肯德基 西坝河店,这样带有空格的,空格不需要解析,但是要用到最后的ex_中,所以不能缺少
			stWTSLast.strWord = strWord + stWTSLast.strWord;
			continue;
		}
		vector<string> vecFull;
		vector<string> vecSimple;
		//获取全拼和简拼结果
		Pinyin::getInstance().getMultiPinyinHeadUpper(vecFull, strWord);
		Pinyin::getInstance().getMultiSimplePinyin(vecSimple, strWord);

		//赋值结果
		stWTS.vecFull = vecFull;
		vector<string> vecNull;
		for(jLoop = 0; jLoop < vecSimple.size(); jLoop++) {
			stWTS.mapSimple[vecSimple[jLoop]] = vecNull;
		}
		iSize = vecFull.size();
		//把简拼对应的全拼填写到简拼后面
		for(jLoop = 0; jLoop < iSize; jLoop++) {
			string strSimpleFromFull = "";
			int kLoop;
			for(kLoop = 0; kLoop < vecFull[jLoop].size(); kLoop++) {
				char c = vecFull[jLoop][kLoop];
				if(c >= 'A' && c <= 'Z') {
					c = c - 'A' + 'a';
					strSimpleFromFull += c;
				} else if(c >= '0' && c <= '9') {
					strSimpleFromFull += c;
				}
			}
			map<string, vector<string> >::iterator iIter;
			if((iIter = stWTS.mapSimple.find(strSimpleFromFull)) != stWTS.mapSimple.end()) {
				if(find(iIter->second.begin(), iIter->second.end(), strSimpleFromFull) == iIter->second.end()) {
					iIter->second.push_back(vecFull[jLoop]);
				}
			}
		}
		if(iLoop != vecSplit.size() - 1) { //不是第一次
			stWTS.strWord = stWTS.strWord + stWTSLast.strWord;
			vector<string> vecNewFull;
			string strJoinTmp;
			int ii, jj;
			for(ii = 0; ii < stWTS.vecFull.size(); ii++) {
				for(jj = 0; jj < stWTSLast.vecFull.size(); jj++) {
					strJoinTmp = stWTS.vecFull[ii] + stWTSLast.vecFull[jj];
					vecNewFull.push_back(strJoinTmp);
				}
			}
			stWTS.vecFull = vecNewFull;

			map<string, vector<string> > mapNewSimple;
			map<string, vector<string> >::iterator iIterI, iIterJ;
			for(iIterI = stWTS.mapSimple.begin(); iIterI != stWTS.mapSimple.end(); iIterI++) {
				for(iIterJ = stWTSLast.mapSimple.begin(); iIterJ != stWTSLast.mapSimple.end(); iIterJ++) {
					strJoinTmp = iIterI->first + iIterJ->first;
					if(iIterJ->second.size() == 0) { //如果上次获取的简拼对应的全拼没有获取到
						mapNewSimple[strJoinTmp] = iIterI->second;
					} else { //获取到了，则合并
						vector<string> vecFullTmp;
						string strTmp2;
						for(ii = 0; ii < iIterI->second.size(); ii++) {
							for(jj = 0; jj < iIterJ->second.size(); jj++) {
								strTmp2 = iIterI->second[ii] + iIterJ->second[jj];
								vecFullTmp.push_back(strTmp2);
							}
						}
						mapNewSimple[strJoinTmp] = vecFullTmp;
					}
				}
			}
			stWTS.mapSimple = mapNewSimple;
		}
		vecPinyinTmp.push_back(stWTS);
		stWTSLast = stWTS;
	}
	//反置
	for(iLoop = vecPinyinTmp.size() - 1; iLoop >= 0; iLoop--) {
		vecPinyin.push_back(vecPinyinTmp[iLoop]);
	}
#if 0 //debug
	//---------0
	//string-->店
	//--simple:d-->>Dian;
	//--full:1-->>Dian;
	//---------1
	//string-->鼓楼南街店
	//--simple:glnjd-->>GuLouNanJieDian;
	//--full:1-->>GuLouNanJieDian;
	//---------2
	//string-->肯德基 鼓楼南街店
	//--simple:kdjglnjd-->>KenDeJiGuLouNanJieDian;
	//--full:1-->>KenDeJiGuLouNanJieDian;
	for(int iLoop = 0; iLoop < vecPinyinTmp.size(); iLoop++) {
		printf("\n---------%d\n", iLoop);
		WordToSpell &WTSTmp = vecPinyinTmp[iLoop];
		printf("string-->%s\n", WTSTmp.strWord.c_str());
		map<string, vector<string> > & mapT = WTSTmp.mapSimple;
		map<string, vector<string> >::iterator iIterMap;
		for(iIterMap = mapT.begin(); iIterMap != mapT.end(); iIterMap++) {
			printf("--simple:%s-->>", iIterMap->first.c_str());
			for(int jLoop = 0; jLoop < iIterMap->second.size(); jLoop++) {
				printf("%s;", iIterMap->second[jLoop].c_str());
			}
			printf("\n");
		}
		printf("--full:%d-->>", WTSTmp.vecFull.size());
		for(int jLoop = 0; jLoop < WTSTmp.vecFull.size(); jLoop++) {
			printf("%s;", WTSTmp.vecFull[jLoop].c_str());
		}
		printf("\n\n");
	}
#endif
}

//strPre: 前缀
//strExPre: 扩展使用的前缀
//strKey:   关键词 
//writeFds: 要写得文件句柄
//iWeight: 当前语料的权重
//iCount:  当前处理的是汉字词典中的第几行
int CCreateDict::WriteDictFirst(const string & strPre, const string & strExPre, const string & strKey, CWriteBigFile * & writeFds, const int iWeight, const int iCount)
{
	vector<WordToSpell> vecPinyin;
	if(m_bNeedPinyinDeal) {
		SplitWordWithPinyin(strKey, vecPinyin);
	} else {
		SplitWordNoPinyin(strKey, vecPinyin);
	}
	//if(m_bDictNeedCreate[SELECT_TREE_EX_CHINESE]) { //扩展汉字
	//	WriteOneDict(strExPre, strKey, writeFds[SELECT_TREE_EX_CHINESE], iWeight, iCount);
	//}
	int iLoop, jLoop;
	int iSize = vecPinyin.size();
	string strKeySplit = "";
	for(iLoop = iSize - 1; iLoop > 0; iLoop--) { //这里判断大于0因为全部的字符串不加到中间查询中
		strKeySplit = vecPinyin[iLoop].strWord;
		//个数和长度限制
		if(iLoop > m_iMaxSplitNum || strKeySplit.size() < m_iMinSplitCenterWordLen) {
			continue;
		}
		vector<string> & vecF = vecPinyin[iLoop].vecFull;
		string strSpell = "";
		for(jLoop = 0; jLoop < vecF.size(); jLoop++) {
			if(jLoop == 0) {
				strSpell = vecF[jLoop];
			} else {
				strSpell += "," + vecF[jLoop];
			}
		}
		if(m_bDictNeedCreate[SELECT_TREE_CENTER_CHINESE]) { //中间汉字
			WriteOneDict(strPre, strKeySplit, writeFds[SELECT_TREE_CENTER_CHINESE], iWeight, iCount, strSpell);
		}
		if(m_bDictNeedCreate[SELECT_TREE_EX_CENTER_CHINESE]) { //扩展中间汉字
			WriteOneDict(strExPre, strKeySplit, writeFds[SELECT_TREE_EX_CENTER_CHINESE], iWeight, iCount, strSpell);
		}
		for(jLoop = 0; jLoop < vecF.size(); jLoop++) {
			if(m_bDictNeedCreate[SELECT_TREE_CENTER_SPELL]) { //中间拼音
				WriteOneDict(strPre, vecF[jLoop], writeFds[SELECT_TREE_CENTER_SPELL], iWeight, iCount, strKeySplit);
			}
			if(m_bDictNeedCreate[SELECT_TREE_EX_CENTER_SPELL]) { //扩展中间拼音
				WriteOneDict(strExPre, vecF[jLoop], writeFds[SELECT_TREE_EX_CENTER_SPELL], iWeight, iCount, strKeySplit);
			}
		}
		if(m_bDictNeedCreate[SELECT_TREE_CENTER_SIMPLE] || m_bDictNeedCreate[SELECT_TREE_EX_CENTER_SIMPLE]) {
			map<string, vector<string> > & mapT = vecPinyin[iLoop].mapSimple;
			map<string, vector<string> >::iterator iIterM;
			for(iIterM = mapT.begin(); iIterM != mapT.end(); iIterM++) {
				string strExtra2 = "";
				for(jLoop = 0; jLoop < iIterM->second.size(); jLoop++) {
					string strT = iIterM->second[jLoop];
					if(strT != "") {
						if(strExtra2 != "") {
							strExtra2 += ",";
						}
						strExtra2 += strT;
					}
				}
				if(m_bDictNeedCreate[SELECT_TREE_CENTER_SIMPLE]) { //中间拼音(简拼)
					WriteOneDict(strPre, iIterM->first, writeFds[SELECT_TREE_CENTER_SIMPLE], iWeight, iCount, strExtra2);
				}
				if(m_bDictNeedCreate[SELECT_TREE_EX_CENTER_SIMPLE]) { //扩展中间拼音(简拼)
					WriteOneDict(strExPre, iIterM->first, writeFds[SELECT_TREE_EX_CENTER_SIMPLE], iWeight, iCount, strExtra2);
				}
			}
		}
	}
	if(iSize > 0) {
		strKeySplit = vecPinyin[0].strWord;
		if(m_bDictNeedCreate[SELECT_TREE_EX_CHINESE]) { //扩展汉字
			WriteOneDict(strExPre, strKeySplit, writeFds[SELECT_TREE_EX_CHINESE], iWeight, iCount);
		}
		vector<string> & vecF = vecPinyin[0].vecFull; 
		for(jLoop = 0; jLoop < vecF.size(); jLoop++) {
			if(m_bDictNeedCreate[SELECT_TREE_SPELL]) { //拼音
				WriteOneDict(strPre, vecF[jLoop], writeFds[SELECT_TREE_SPELL], iWeight, iCount);
			}
			if(m_bDictNeedCreate[SELECT_TREE_EX_SPELL]) { //扩展拼音
				WriteOneDict(strExPre, vecF[jLoop], writeFds[SELECT_TREE_EX_SPELL], iWeight, iCount);
			}
		}
		map<string, vector<string> > & mapT = vecPinyin[0].mapSimple;
		map<string, vector<string> >::iterator iIterM;
		if(m_bDictNeedCreate[SELECT_TREE_SIMPLE] || m_bDictNeedCreate[SELECT_TREE_EX_SIMPLE]) {
			for(iIterM = mapT.begin(); iIterM != mapT.end(); iIterM++) {
				string strExtra2 = "";
				for(jLoop = 0; jLoop < iIterM->second.size(); jLoop++) {
					string strT = iIterM->second[jLoop];
					if(strT != "") {
						if(strExtra2 != "") {
							strExtra2 += ",";
						}
						strExtra2 += strT;
					}
				}
				if(m_bDictNeedCreate[SELECT_TREE_SIMPLE]) { //拼音(简拼)
					WriteOneDict(strPre, iIterM->first, writeFds[SELECT_TREE_SIMPLE], iWeight, iCount, strExtra2);
				}
				if(m_bDictNeedCreate[SELECT_TREE_EX_SIMPLE]) { //扩展拼音(简拼)
					WriteOneDict(strExPre, iIterM->first, writeFds[SELECT_TREE_EX_SIMPLE], iWeight, iCount, strExtra2);
				}
			}
		}
	}
	return 0;
}
int CCreateDict::FormatJoinOneFile(const int iFieldId, map<const string, RecordInfo> & mapRecord)
{
	int iLoop;
	CReadBigFile fdR;
	string strFile = m_strDataPath + "/" + m_strTaskName + "/" + m_pSysConfigSet->GetFileName(iFieldId) + m_strTmpSuffix;
	fdR.Open(strFile.c_str());
	map<const string, RecordInfo>::iterator iIterMap;
	char *p;
	while(fdR.GetLine(p)) {
		vector<string> vecSplit, vecSplit2;
		XHStrUtils::StrTokenizeGBK(vecSplit, p, "|");
		if(vecSplit.size() < 2) { //严重错误
			printf("error:%s:%d:%d\n", p, iFieldId, vecSplit.size());
			continue;
			exit(-1);
		}
		XHStrUtils::StrTokenizeGBK(vecSplit2, vecSplit[0], ",");
		if(vecSplit2.size() != 3) {
			printf("error:2:%s:%d:%d\n", p, iFieldId, vecSplit2.size());
			continue;
			exit(-1);
		}

		vector<string> vecSplit3;
		if(vecSplit.size() == 3) { //扩展2
			XHStrUtils::StrTokenizeGBK(vecSplit3, vecSplit[2], ",");
		}
		string strKey = vecSplit2[0] + "," + vecSplit2[1];
		iIterMap = mapRecord.find(strKey);
		if(iIterMap == mapRecord.end()) {
			RecordInfo stTmp;
			stTmp.iWeight = atoi(vecSplit2[2].c_str());
			stTmp.vecIds.push_back(vecSplit[1]);
			for(iLoop = 0; iLoop < vecSplit3.size(); iLoop++) {
				if(find(stTmp.vecExtra2.begin(), stTmp.vecExtra2.end(), vecSplit3[iLoop]) == stTmp.vecExtra2.end()) { //没有找到
					stTmp.vecExtra2.push_back(vecSplit3[iLoop]);
				}
			}
			mapRecord[strKey] = stTmp;
		} else { //已经增加过了
			int iWeight = atoi(vecSplit2[2].c_str());
			if(iIterMap->second.iWeight < iWeight) {
				iIterMap->second.iWeight = iWeight;
			}
			iIterMap->second.vecIds.push_back(vecSplit[1]);
			for(iLoop = 0; iLoop < vecSplit3.size(); iLoop++) {
				if(find(iIterMap->second.vecExtra2.begin(), iIterMap->second.vecExtra2.end(), vecSplit3[iLoop]) == iIterMap->second.vecExtra2.end()) { //没有找到
					iIterMap->second.vecExtra2.push_back(vecSplit3[iLoop]);
				}
			}
		}
	}
	return 0;
}

//输出一个词典文件，覆盖第一次生成的文件
int CCreateDict::WriteJoinOneFile(const int iFieldId, map<const string, RecordInfo> & mapRecord)
{
	CWriteBigFile fdW;
	string strFile = m_strDataPath + "/" + m_strTaskName + "/" + m_pSysConfigSet->GetFileName(iFieldId);
	fdW.Open(strFile.c_str());
	map<const string, RecordInfo>::iterator iIter;
	char szInfo[256];
	int iLoop;
	int iType;
	//便于把扩展的压缩到非扩展的数据里面
	//把含有多个结果的放到前面
	for(iType = 0; iType < 2; iType++) {
		for(iIter = mapRecord.begin(); iIter != mapRecord.end(); iIter++) {
			if((iType == 1 && iIter->second.vecIds.size() > 1) || (iType == 0 && iIter->second.vecIds.size() == 1)) {
				continue;
			}
			string strResult = iIter->first;
			snprintf(szInfo, sizeof(szInfo) / sizeof(szInfo[0]), ",%d|", iIter->second.iWeight);
			strResult += szInfo;
			for(iLoop = 0; iLoop < iIter->second.vecIds.size(); iLoop++) {
				if(iLoop == 0) {
					strResult += iIter->second.vecIds[iLoop];
				} else {
					strResult += ",";
					strResult += iIter->second.vecIds[iLoop];
				}
			}
			string strTmp = "";
			for(iLoop = 0; iLoop < iIter->second.vecExtra2.size(); iLoop++) {
				if(iLoop == 0) {
					strTmp = iIter->second.vecExtra2[iLoop];
				} else {
					strTmp += ",";
					strTmp += iIter->second.vecExtra2[iLoop];
				}
			}
			if(strTmp != "") {
				map<const string, int> mapTmp;
				map<const string, int>::iterator iIterM;
				vector<string> vecSplit;
				XHStrUtils::StrTokenizeGBK(vecSplit, strTmp, ",");
				for(int kLoop = 0; kLoop < vecSplit.size(); kLoop++) {
					mapTmp[vecSplit[kLoop]] = 1;
				}
				strTmp = "";
				for(iIterM = mapTmp.begin(); iIterM != mapTmp.end(); iIterM++) {
					if(strTmp == "") {
						strTmp += "|";
					} else {
						strTmp += ",";
					}
					strTmp += iIterM->first;
				}
				strResult += strTmp;
			}
			fdW.WriteLine(strResult.c_str());
		}
	}

	return 0;
}

//把每个文件都进行组合，分散在不同行的数据合在一起
int CCreateDict::CreateAllDict_Second()
{
	int iLoop;
	for(iLoop = 0; iLoop < DICT_MAX_SIZE; iLoop++) {
		map<const string, RecordInfo> mapRecord;
		if(m_bDictNeedCreate[iLoop]) {
			printf("task name:%s; dict id:%d\n", m_strTaskName.c_str(), iLoop);
			FormatJoinOneFile(iLoop, mapRecord);
			WriteJoinOneFile(iLoop, mapRecord);
		}
	}
	return 0;
}
//第一次生成全部的字典（可能有重复，第二遍会去掉重复)
int CCreateDict::CreateAllDict_First()
{
	char *p;
	int iCount = -1;
	int iLoop;
	bool bFirst = true;
	string strEXPre = "";

	CReadBigFile fdR;
	CWriteBigFile fdWs[DICT_MAX_SIZE];
	CWriteBigFile * pfdWs = fdWs;
	string strDictChineseFile = m_strDataPath + "/" + m_strTaskName + "/" + m_pSysConfigSet->GetFileName(SELECT_TREE_CHINESE);
	fdR.Open(strDictChineseFile.c_str());

	//打开所有要写得文件
	for(iLoop = 0; iLoop < DICT_MAX_SIZE; iLoop++) {
		if(m_bDictNeedCreate[iLoop]) {
			string strFile = m_strDataPath + "/" + m_strTaskName + "/" + m_pSysConfigSet->GetFileName(iLoop) + m_strTmpSuffix;
			fdWs[iLoop].Open(strFile.c_str());
		}
	}
	while(fdR.GetLine(p)) {
		iCount++;
		if((iCount % 100000) == 0) printf("%d\n", iCount);
		vector<string> vecSplitWord;
		vector<string> vecSplit;
		XHStrUtils::StrTokenizeGBK(vecSplit, p, ",");
		if(vecSplit.size() < 3) { //即使存在错误也会把该行计算在内
			printf("error->line:%d, %s(less than 3 paramter)\n", iCount, p);
			continue;
		}
		string strPre = vecSplit[1];
		string strKey = vecSplit[0];
		if(bFirst) {
			for(iLoop = 0; iLoop < strPre.size(); iLoop++) {
				strEXPre += "0";
			}
			bFirst = false;
		}
		int iWeight = 0;
		if(vecSplit.size() > 3) {
			iWeight = atoi(vecSplit[3].c_str());
		}
		WriteDictFirst(strPre, strEXPre, strKey, pfdWs, iWeight, iCount);
	}
	return 0;
}

int CCreateDict::Process()
{
	CreateAllDict_First();
	CreateAllDict_Second();
	return 0;
}
