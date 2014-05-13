#include "util/DumpPickIndex.h"
#include "util/WriteLog.h"

bool CDumpPickIndex::Dump(const string & strTaskName, CSysConfigSet *const pSysConfigSet)
{
	return CDumpPickIndex::Run(strTaskName, pSysConfigSet);
}
char * CDumpPickIndex::Pick(const string & strFileName)
{
	IndexFiles stIndexFiles;
	int iFd = open(strFileName.c_str(), O_RDONLY);
	if(iFd <= 0) {
		return NULL;
	}
	int iSize = sizeof(IndexFiles);
	if(iSize != read(iFd, &stIndexFiles, iSize)) {
		close(iFd);
		return NULL;
	}
	lseek(iFd, 0, SEEK_SET);
	char * p = new char[stIndexFiles.uiSize];
	if(p == NULL) {
		close(iFd);
		return NULL;
	}
	if(stIndexFiles.uiSize != read(iFd, p, stIndexFiles.uiSize)) {
		delete []p;
		close(iFd);
		return NULL;
	}
	close(iFd);
	return p;
}

void CDumpPickIndex::WriteData(int & iFd, const char * pData, const int iDataSize)
{
	write(iFd, pData, iDataSize);
}
void CDumpPickIndex::WriteFile(const string & strFileName, int & iFd, U32 & uiSize)
{
	char szLog[1024];
	int iFdTmp = open(strFileName.c_str(), O_RDONLY);
	if(iFdTmp == -1) {
		snprintf(szLog, sizeof(szLog), "error:(warning)open file failed(%s)[%s %d]\n", strFileName.c_str(), __FILE__, __LINE__);
		CWriteLog::GetInstance().WriteLog(szLog);
		return;
		//exit(-1);
	}
	U32 uiFileSize;
	read(iFdTmp, &uiFileSize, 4);
	write(iFd, &uiFileSize, 4);
	uiSize += uiFileSize;
	if(uiFileSize > 4) {
		uiFileSize = uiFileSize - 4;
		char * pData = new char[uiFileSize];
		if(pData == NULL) {
			snprintf(szLog, sizeof(szLog), "error:(exit)malloc %d error (%s)[%s %d]\n", uiFileSize, strFileName.c_str(), __FILE__, __LINE__);
			CWriteLog::GetInstance().WriteLog(szLog);
			exit(-1);
		}
		if(uiFileSize != read(iFdTmp, pData, uiFileSize)) {
			snprintf(szLog, sizeof(szLog), "error:(exit)read file failed(%s)[%s %d]\n", strFileName.c_str(), __FILE__, __LINE__);
			CWriteLog::GetInstance().WriteLog(szLog);
			exit(-1);
		}
		write(iFd, pData, uiFileSize);
		SAFE_DELETE_MUL(pData);
	}
	close(iFdTmp);
}


bool CDumpPickIndex::Run(const string & strTaskName, CSysConfigSet * const pSysConfigSet)
{
	string strPath = pSysConfigSet->GetString(PARA_CONFIG_pathdata);
	strPath += "/" + strTaskName + "/";
	string strDictNeed = pSysConfigSet->GetString(PARA_CONFIG_selecttree);
	vector<string> vecDictNeed;
	int iLoop;
	XHStrUtils::StrTokenize(vecDictNeed, strDictNeed, ";");
	IndexFiles stIndexFiles;
	memset(&stIndexFiles, 0x00, sizeof(stIndexFiles));
	for(iLoop = 0; iLoop < vecDictNeed.size(); iLoop++) {
		int iSub = atoi(vecDictNeed[iLoop].c_str());
		if(iSub >= 0 && iSub <= sizeof(stIndexFiles.bSelect) / sizeof(stIndexFiles.bSelect[0])) {
			stIndexFiles.bSelect[iSub] = 1;
		}
	}

	char szLog[1024];
	string strOutIndex = strPath + strTaskName;
	int iFd = open(strOutIndex.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if(iFd == -1) {
		snprintf(szLog, sizeof(szLog), "error:(exit)open file failed(%s)[%s %d]\n", strOutIndex.c_str(), __FILE__, __LINE__);
		CWriteLog::GetInstance().WriteLog(szLog);
		exit(-1);
	}
	U32 uiSize = sizeof(IndexFiles);
	//填写首结构
	CDumpPickIndex::WriteData(iFd, (char *)(&stIndexFiles), uiSize);
	//汉字的字符串必须要写
	stIndexFiles.uiString[SELECT_TREE_CHINESE] = uiSize;
	stIndexFiles.uiString[SELECT_TREE_CENTER_CHINESE] = uiSize;
	stIndexFiles.uiString[SELECT_TREE_EX_CHINESE] = uiSize;
	stIndexFiles.uiString[SELECT_TREE_EX_CENTER_CHINESE] = uiSize;
	string strChineseString = strPath + pSysConfigSet->GetString(PARA_CONFIG_chinesestring);
	printf("aaaaaaaa----->%d\n", uiSize);
	CDumpPickIndex::WriteFile(strChineseString, iFd, uiSize);
	printf("aaaaaaaa2----->%d\n", uiSize);
	
	//简拼字符串
	if(stIndexFiles.bSelect[SELECT_TREE_SIMPLE] || stIndexFiles.bSelect[SELECT_TREE_CENTER_SIMPLE] ||
		stIndexFiles.bSelect[SELECT_TREE_EX_SIMPLE] || stIndexFiles.bSelect[SELECT_TREE_EX_CENTER_SIMPLE]) {
		string strSimpleString = strPath + pSysConfigSet->GetString(PARA_CONFIG_simplestring);
		stIndexFiles.uiString[SELECT_TREE_SIMPLE] = stIndexFiles.uiString[SELECT_TREE_CENTER_SIMPLE] = uiSize;
		stIndexFiles.uiString[SELECT_TREE_EX_SIMPLE] = stIndexFiles.uiString[SELECT_TREE_EX_CENTER_SIMPLE] = uiSize;
		CDumpPickIndex::WriteFile(strSimpleString, iFd, uiSize);

	}
	//全拼字符串
	if(stIndexFiles.bSelect[SELECT_TREE_SPELL] || stIndexFiles.bSelect[SELECT_TREE_CENTER_SPELL] ||
		stIndexFiles.bSelect[SELECT_TREE_EX_SPELL] || stIndexFiles.bSelect[SELECT_TREE_EX_CENTER_SPELL]) {
		string strSpellString = strPath + pSysConfigSet->GetString(PARA_CONFIG_spellstring);
		stIndexFiles.uiString[SELECT_TREE_SPELL] = stIndexFiles.uiString[SELECT_TREE_CENTER_SPELL] = uiSize;
		stIndexFiles.uiString[SELECT_TREE_EX_SPELL] = stIndexFiles.uiString[SELECT_TREE_EX_CENTER_SPELL] = uiSize;
		CDumpPickIndex::WriteFile(strSpellString, iFd, uiSize);
	}

	//nodedata->>汉字必须要有nodedata
	//extra-->>同时写扩展的信息，扩展和nodedata是一样的
	CDumpPickIndex::WriteNodeData(SELECT_TREE_CHINESE,        SELECT_TREE_EX_CHINESE,        iFd, uiSize, pSysConfigSet, stIndexFiles, true, strPath);
	CDumpPickIndex::WriteNodeData(SELECT_TREE_SPELL,          SELECT_TREE_EX_SPELL,          iFd, uiSize, pSysConfigSet, stIndexFiles, false, strPath);
	CDumpPickIndex::WriteNodeData(SELECT_TREE_SIMPLE,         SELECT_TREE_EX_SIMPLE,         iFd, uiSize, pSysConfigSet, stIndexFiles, false, strPath);
	CDumpPickIndex::WriteNodeData(SELECT_TREE_CENTER_CHINESE, SELECT_TREE_EX_CENTER_CHINESE, iFd, uiSize, pSysConfigSet, stIndexFiles, false, strPath);
	CDumpPickIndex::WriteNodeData(SELECT_TREE_CENTER_SPELL,   SELECT_TREE_EX_CENTER_SPELL,   iFd, uiSize, pSysConfigSet, stIndexFiles, false, strPath);
	CDumpPickIndex::WriteNodeData(SELECT_TREE_CENTER_SIMPLE,  SELECT_TREE_EX_CENTER_SIMPLE,  iFd, uiSize, pSysConfigSet, stIndexFiles, false, strPath);

	//treenode
	for(iLoop = 0; iLoop < DICT_MAX_SIZE; iLoop++) {
		if(!stIndexFiles.bSelect[iLoop]) {
			continue;
		}
		stIndexFiles.uiTreeNode[iLoop] = uiSize;
		string strFileName = strPath + pSysConfigSet->GetFileName(iLoop) + pSysConfigSet->GetString(PARA_CONFIG_suffix_treenode);
		CDumpPickIndex::WriteFile(strFileName, iFd, uiSize);
	}

	//mixsearch
	int iWithMix = pSysConfigSet->GetInt(PARA_CONFIG_withmixsearch);
	if(iWithMix) {
		for(iLoop = 0; iLoop < DICT_MAX_SIZE; iLoop++) {
			if(!stIndexFiles.bSelect[iLoop]) {
				continue;
			}
			//汉字没有混合
			if(iLoop == SELECT_TREE_CHINESE || iLoop == SELECT_TREE_EX_CHINESE || iLoop == SELECT_TREE_CENTER_CHINESE || iLoop == SELECT_TREE_EX_CENTER_CHINESE) {
				continue;
			}
			stIndexFiles.uiMixSearch[iLoop] = uiSize;
			string strMixSearch = strPath + pSysConfigSet->GetFileName(iLoop) + pSysConfigSet->GetString(PARA_CONFIG_suffix_mixsearch);
			printf("---------%s\n", strMixSearch.c_str());
			CDumpPickIndex::WriteFile(strMixSearch, iFd, uiSize);
			if(uiSize - stIndexFiles.uiMixSearch[iLoop] <= sizeof(int)) {
				stIndexFiles.uiMixSearch[iLoop] = 0;
			}
		}
	}
	stIndexFiles.uiSize = uiSize;
	lseek(iFd, 0, SEEK_SET);
	WriteData(iFd, (char *)(&stIndexFiles), sizeof(stIndexFiles));
	close(iFd);

	return true;
}

void CDumpPickIndex::WriteNodeData(const int iSelect1, const int iSelect2, int & iFd, U32 &uiSize, CSysConfigSet *const pSysConfigSet, IndexFiles &stIndexFiles, const bool bMustAdd, const string &strPath)
{
	if(!(stIndexFiles.bSelect[iSelect1] || stIndexFiles.bSelect[iSelect2] || bMustAdd)) {
		return ;
	}
	string strNodeData = strPath + pSysConfigSet->GetFileName(iSelect1) + pSysConfigSet->GetString(PARA_CONFIG_suffix_nodedata);
	stIndexFiles.uiNodeData[iSelect1] = stIndexFiles.uiNodeData[iSelect2] = uiSize;
	CDumpPickIndex::WriteFile(strNodeData, iFd, uiSize);
	string strExtra = strPath + pSysConfigSet->GetFileName(iSelect1) + pSysConfigSet->GetString(PARA_CONFIG_suffix_extra);
	stIndexFiles.uiExtra[iSelect1] = stIndexFiles.uiExtra[iSelect2] = uiSize;
	CDumpPickIndex::WriteFile(strExtra, iFd, uiSize);
}
