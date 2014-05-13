#include "util/CompressPinyin.h"
#include "util/XHStrUtils.h"
#include "common/SysCommon.h"


CCompressPinyin::CCompressPinyin()
{
	Init();
}

CCompressPinyin & CCompressPinyin::GetInstance()
{
	static CCompressPinyin p;
	return p;
}

void CCompressPinyin::Init()
{
	int iTableSize = sizeof(pyTable) / sizeof(pyTable[0]);
	int iLoop;
	char cSubPos = 0;
	for(iLoop = 0; iLoop < MAX_CUT_PINYIN_NUMBER; iLoop++) m_strPosToString[cSubPos] = "";
	for(iLoop = 0; iLoop < iTableSize; iLoop++) {
		string strTmp = pyTable[iLoop];
		if(strTmp.size() == 0) continue;
		strTmp = strTmp.substr(1);
		if(m_mapSpellPos.find(strTmp) != m_mapSpellPos.end()) {
			continue;
		}
		m_strPosToString[cSubPos] = strTmp;
		m_mapSpellPos[strTmp] = cSubPos;
		cSubPos++;
	}
}

bool CCompressPinyin::CompressStr(const int iNums, const string & strFull, char * pzRes)
{
	string strResult = "";
	int iSize = strFull.size();
	int jLoop = 0;
	int iLoop;
	map<const string, char>::iterator iIter;
	for(iLoop = 0; iLoop < iSize && jLoop < iNums; ) {
		string strTmp = "";
		if(strFull[iLoop] >= 'A' && strFull[iLoop] <= 'Z') {
			if(iLoop == iSize - 1) {
				pzRes[jLoop] = m_mapSpellPos[""];
				jLoop++;
				break;
			}
		} else if(strFull[iLoop] >= '0' && strFull[iLoop] <= '9') {
			pzRes[jLoop] = m_mapSpellPos[""];
			jLoop++;
			continue;
		} else {
			return "";
		}
		for(iLoop++; iLoop < iSize; iLoop++) {
			if(strFull[iLoop] >= 'A' && strFull[iLoop] <= 'Z' || strFull[iLoop] >= '0' && strFull[iLoop] <= '9') {
				break;
			}
			strTmp += strFull[iLoop];
		}
		iIter = m_mapSpellPos.find(strTmp);
		if(iIter == m_mapSpellPos.end()) return false;
		pzRes[jLoop] = iIter->second;
		jLoop++;
	}
	if(jLoop == iNums) return true;
	return false;
}


bool CCompressPinyin::UnCompressStr(const string strSimple, const char * pzRes, string & strFull)
{
	int iSize = strSimple.size();
	int iLoop;
	strFull = "";
	for(iLoop = 0; iLoop < iSize; iLoop++) {
		char c = strSimple[iLoop];
		if(!(c >= 'a' && c <= 'z')) {
			return false;
		}
		c = c - 'a' + 'A';
		strFull += c;
		strFull += m_strPosToString[pzRes[iLoop]];
	}
	return true;
}
bool CCompressPinyin::PacketMixSearch(vector<pair<int, string> > & vecData, char * pzRet, const int iMaxResSize, int &iRetSize)
{
	int iSize = vecData.size();
	int iLoop, jLoop;
	int iPos = 0;
	for(iLoop = 0; iLoop < iSize; iLoop++) {
		int iLen = vecData[iLoop].first;
		string strData = vecData[iLoop].second;
		if(iLen > 0xff) {
			continue;
		}
		pzRet[iPos] = iLen & 0xff;
		vector<string> vecSplit;
		XHStrUtils::StrTokenize(vecSplit, strData, "|");

		int iMove = iPos + 1;
		for(jLoop = 0; jLoop < vecSplit.size(); jLoop++) {
			if(iMove + 100 > iMaxResSize) return false;
			if(CompressStr(iLen, vecSplit[jLoop], pzRet + iMove)) {
				iMove += iLen;
			}
		}
		if(iMove != iPos + 1) {
			pzRet[iMove] = SPLIT_MUL_STR;
			iMove++;
			iPos = iMove;
		} else {
			printf("error:(%s:%d) PacketMixSearch[%s %d]\n", strData.c_str(), iLen, __FILE__, __LINE__);
		}
	}
	if(iPos == 0) {
		return false;
	}
	pzRet[iPos] = ALL_OVER_CHAR;
	iPos++;
	iRetSize = iPos;
	return true;
}


bool CCompressPinyin::GetMatchStrs(const char * cValue, vector<string> & vecRet)
{
	string strValue = "";
	int kLoop = 0;
	while(1) {
		if(cValue[kLoop] != '~' && cValue[kLoop] != 0) {
			strValue += cValue[kLoop];
		} else {
			break;
		}
		kLoop++;
	}
	XHStrUtils::StrTokenize(vecRet, strValue, "|");
	return true;
}
bool CCompressPinyin::GetMatchStrs(const string & strSimple, const char * pOrigStr, const char * pMatchedStr, const string & strTail, vector<string> & vecRet)
{
	const char * pTmp = pMatchedStr;
	int iLoop;
	int iSimpleSize = strSimple.size();
	char c = strSimple[iSimpleSize - 1];
	if(c >= 'a' && c <= 'z') c = c - 'a' + 'A';
	string strSimplePart = strSimple.substr(0, iSimpleSize - 1);
	//返回的指针指向个数,往后移动一个字节才是真正的内容
	int iMoveSize = pTmp[0];
	pTmp++;
	while(1) {
		if(pTmp[0] == (char)SPLIT_MUL_STR || pTmp[0] == (char)ALL_OVER_CHAR) {
			break;
		}   
		for(iLoop = 0; iLoop < iSimpleSize - 1; iLoop++) {
			if(pOrigStr[iLoop] != 0 && pOrigStr[iLoop] != pTmp[iLoop]) {
				break;
			}
		}   
		if(iLoop != iSimpleSize - 1) {
			pTmp += iMoveSize;
			continue;
		}   
		string strTail2 = m_strPosToString[pTmp[iSimpleSize - 1]];
		if((strTail.size() < strTail2.size() && memcmp(strTail.c_str(), strTail2.c_str(), strTail.size()) == 0) || strTail == strTail2) { //匹配上
			string strFull;
			if(UnCompressStr(strSimplePart, pTmp, strFull)) {
				strFull += c;
				strFull += strTail;
				//printf("full=================================================================%s\n", strFull.c_str());
				vecRet.push_back(strFull);
			}
		}
		pTmp += iMoveSize;
	}
	return true;
}
