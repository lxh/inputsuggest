#include "isserver/PacketResult.h"


CPacketResult::CPacketResult()
{
}

string CPacketResult::Packet(TopResult & topResult)
{
	string strResult = "";
    ResultElement * pRE;
    char szStr[10240];
	int iNumber = topResult.getSavedSize();
    while(pRE = (ResultElement*)topResult.tipqueue.pop()) {
        CTreeChinese *pChinese = (CTreeChinese *)pRE->pPara;
        const XHChineseData * pCD = (XHChineseData *)pChinese->GetNodeDataFromId(pRE->eleId);
        const char * pStr = (const char *)pChinese->GetNodeStringFromData(pCD);
        const char * pOrigStr = (const char *)pChinese->GetNodeOrigStringFromData(pCD);
        const char * pExtra = (const char *)pChinese->GetNodeExtraStringFromData(pCD);
		if(pOrigStr == NULL) {
			pOrigStr = "";
		}
        if(pExtra) {
			snprintf(szStr, sizeof(szStr), "{\"str\":\"%s\",\"orig\":\"%s\",\"weight\":\"%d\",\"flag\":\"%d(%d)\",\"querynum\":\"%d\",\"extra\":\"%s\"},\n", pStr, pOrigStr, pCD->usWeight, pCD->usAttr, pRE->eleId, pRE->queryNum, pExtra);
        } else {
			snprintf(szStr, sizeof(szStr), "{\"str\":\"%s\",\"orig\":\"%s\",\"weight\":\"%d\",\"flag\":\"%d\",\"querynum\":\"%d\"},\n", pStr, pOrigStr, pCD->usWeight, pCD->usAttr, pRE->queryNum);
        }
		strResult = szStr + strResult;
        delete pRE;
    }
	if(strResult.size() > 2) {
		strResult = strResult.substr(0,strResult.size() - 2);
	}
	char szNumber[50];
	snprintf(szNumber, sizeof(szNumber), "\"number\":\"%d\",", iNumber);
	string strNumber = szNumber;
	strResult = strNumber + "\"detailall\":[\n" + strResult +  "\n]\n";
    return strResult;
}
