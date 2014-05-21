#include "isindex/IndexDeleteItem.h"

int CIndexDeleteItem::Run(const string & strTaskName, CSysConfigSet *pclsSysConfigSet)
{
	m_strTaskName = strTaskName;
	m_pSysConfigSet = pclsSysConfigSet;
	return Process();
}

int CIndexDeleteItem::Process()
{
	string strDataPath   = m_pSysConfigSet->GetString(PARA_CONFIG_pathdata);
	string strTaskPath   = strDataPath + "/" + m_strTaskName + "/";
	string strFileDel    = strTaskPath + "chinese.dict.del";
	string strFileDict00 = strTaskPath + "dict.00";
	string strFileDictND = strTaskPath + "dict.00" + m_pSysConfigSet->GetString(PARA_CONFIG_suffix_nodedata);
	vector<string> vecDelData;
	char * pDictND = NULL;
	unsigned int uiSize;
	if(Process_GetDelMap(strFileDel, vecDelData) == false) return 0;
	if(CFileCommon::ReadOneFile(strFileDictND, pDictND, uiSize) == false) return 0;
	bool bRet = Process_WriteNodeData(strFileDict00, vecDelData, pDictND);
	if(bRet) {
		CFileCommon::WriteOneFile(strFileDictND, pDictND + 4, uiSize - 4);
	}

	SAFE_DELETE_MUL(pDictND);
	printf("CIndexDeleteItem::Process --->> ok\n");
	return 1;
}

bool CIndexDeleteItem::Process_WriteNodeData(const string & strFileName, vector<string> & vecDel, char * & pDictND)
{
	CReadBigFile fdR;
	fdR.Open(strFileName.c_str());
	char *p;
	int iLine = -1;
	int iLoop;
	char szBody[102400];
	bool bChange = false;
	while(fdR.GetLine(p)) {
		iLine++;
		//查找是否有|,扩展信息
		for(iLoop = 0; p[iLoop] != '|' && p[iLoop] != 0; iLoop++) {
			szBody[iLoop] = p[iLoop];
			if(p[iLoop] & 0x80) {
				iLoop++;
				szBody[iLoop] = p[iLoop];
			}
		}
		if(p[iLoop] == '|') { //含有扩展信息
			szBody[iLoop] = 0;
		}
		vector<string> vecSplit;
		XHStrUtils::StrTokenizeGBK(vecSplit, szBody, ",");
		if(vecSplit.size() <= 3) {
			continue;
		}
		string strNew = vecSplit[1] + "#" + vecSplit[0];
		if(find(vecDel.begin(), vecDel.end(), strNew) != vecDel.end()) {
			//printf("del->%s\n", p);
			XHChineseData * pXHCD = (XHChineseData*)(pDictND + 4 + sizeof(XHChineseData) * iLine);
			pXHCD->ucAttr |= FLAG_CHINESE_UCATTR_DELETE;
			bChange = true;
		}
	}
	return bChange;
}

//获取名称 前缀 放到vector中
//如果文件不存在或者为空,则返回false
bool CIndexDeleteItem::Process_GetDelMap(const string & strFileName, vector<string> & vecDel)
{
	CReadBigFile fdR;
	if(!fdR.Open(strFileName.c_str())) {
		return false;
	}
	char *p;
	int iLine = 0;
	int iLoop;
	char szBody[102400];
	while(fdR.GetLine(p)) {
		//查找是否有|,扩展信息
		for(iLoop = 0; p[iLoop] != '|' && p[iLoop] != 0; iLoop++) {
			szBody[iLoop] = p[iLoop];
			if(p[iLoop] & 0x80) {
				iLoop++;
				szBody[iLoop] = p[iLoop];
			}
		}
		if(p[iLoop] == '|') { //含有扩展信息
			szBody[iLoop] = 0;
		}
		vector<string> vecSplit;
		XHStrUtils::StrTokenizeGBK(vecSplit, szBody, ",");
		if(vecSplit.size() <= 2) {
			continue;
		}
		string strNew = vecSplit[0] + "#" + vecSplit[1];
		vecDel.push_back(strNew);
		iLine++;
	}
	if(iLine == 0) {
		printf("no delete file or the file is empty[%s]\n", strFileName.c_str());
		return false;
	}
	sort(vecDel.begin(), vecDel.end());
	return true;
}
