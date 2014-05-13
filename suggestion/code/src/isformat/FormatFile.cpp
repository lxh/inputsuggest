#include "isformat/FormatFile.h"
#include "util/ConfigReader.h"
#include "util/ReadBigFile.h"
#include "util/WriteBigFile.h"
#include "util/XHStrUtils.h"

#define UP_WIRTE_MAX 1000

int CFormatFile::RunFormat(const string & strFileIn, const string & strPath, const string & strFileOut, int iMustWrite)
{
	printf("path:%s; filein:%s; fileout:%s\n", strPath.c_str(), strFileIn.c_str(), strFileOut.c_str());
	string strPathNew = strPath;
	int iPos = strPath.size() - 1;
	for(; iPos > 0; iPos--) {
		if(strPath[iPos] != '\\') {
			break;
		}
	}
	if(iPos == 0) {
		printf("error: RunFormat path error:%s\n", strPath.c_str());
		return -1;
	}
	strPathNew = strPathNew.substr(0, iPos + 1);
	m_strFileIn  = strPathNew + "/" + strFileIn;
	m_strFileOut = strPathNew + "/" + strFileOut;

	m_iMustWrite = iMustWrite;
	return Process();
}

int CFormatFile::Process()
{
	CReadBigFile fdR;
	CWriteBigFile fdW;
	CWriteBigFile fdWError;
	string strError = m_strFileIn + ".error";
	fdR.Open(m_strFileIn.c_str());
	fdW.Open(m_strFileOut.c_str());
	fdWError.Open(strError.c_str());
	int iM = m_iMustWrite / 2;
	int iMLoop = 0;

	char *p;
	map<string, int> mapHasCollected;
	while(fdR.GetLine(p)) {
		vector<string> vecSplit;
		XHStrUtils::StrTokenize(vecSplit, p, ",");
		if(vecSplit.size() < 2) {
			string strErrMsg = "error:(less than 2 paramter)";
			strErrMsg += p;
			fdWError.WriteLine((const char *)strErrMsg.c_str());
			continue;
		}
		string strValue = vecSplit[1];
		XHStrUtils::FiltNoiseSymbol(strValue); //ȥ������
		XHStrUtils::ToCase(strValue, false);   //ת����Сд
		XHStrUtils::JoinMultiSpace(strValue);  //�Ѷ���ո�ϲ���һ��
		XHStrUtils::trim(strValue);            //ȥ���ַ������˵Ŀո�
		if(strValue.size() < 2) {
			string strErrMsg = "error:(after format, less than 2 charater)";
			strErrMsg += p;
			fdWError.WriteLine((const char *)strErrMsg.c_str());
			continue;
		}
		string strPreAndValue = vecSplit[0] + "#" + strValue;
		if(mapHasCollected.find(strPreAndValue) != mapHasCollected.end()) {
			if(m_iMustWrite) { //ǿ�ư��ظ��ķŽ�ȥ
				int mLoop = 0;
				for(; mLoop < iM; mLoop++) {
					int iNewId = m_iMustWrite + iMLoop; //�����ظ������ظ��ģ�䣬�ӿ��ٶ�
					iMLoop++;
					char szWrite[512];
					snprintf(szWrite, sizeof(szWrite), "%d", iNewId);
					strPreAndValue = szWrite;
					strPreAndValue += "#" + strValue;
					if(mapHasCollected.find(strPreAndValue) == mapHasCollected.end()) {
						mapHasCollected[strPreAndValue] = 1;
						string strResult = "";
						//�Ѹ�ʽ���õ��µķŵ���ǰ��
						strResult = strValue + "," + szWrite;
						for(int iLoop = 1; iLoop < vecSplit.size(); iLoop++) {
							strResult += "," + vecSplit[iLoop];
						}
						fdW.WriteLine((const char *)strResult.c_str());
						break;
					}
				}
				if(iMLoop > iM) iMLoop=0;
				if(mLoop == iM) {
					string strErrMsg = "error:(has collected key)";
					strErrMsg += p;
					fdWError.WriteLine((const char *)strErrMsg.c_str());
					continue;
				}
				continue;
			} else {
				string strErrMsg = "error:(has collected key)";
				strErrMsg += p;
				fdWError.WriteLine((const char *)strErrMsg.c_str());
				continue;
			}
		}
		mapHasCollected[strPreAndValue] = 1;
		string strResult = "";
		//�Ѹ�ʽ���õ��µķŵ���ǰ��
		strResult = strValue + ",";
		strResult += p;
		fdW.WriteLine((const char *)strResult.c_str());
	}                     
	return 0;
}
