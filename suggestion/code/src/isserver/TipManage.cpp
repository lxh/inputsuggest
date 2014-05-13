#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <iconv.h>
#include "util/XHStrUtils.h"
#include "isserver/TipManage.h"
#include "common/TipServerParaIntro.h"
#include "isserver/TopResult.h"

#define ONE_SEACH_MAXTIME 1 //һ����ѯ������õ�ʱ��

CTipManage::CTipManage()
{
	//m_pmtMutex = PTHREAD_MUTEX_INITIALIZER;
    m_iQueryThreadNum = G_INT(querythreadnum);
	m_pulArrayThreadCalled = new unsigned long[m_iQueryThreadNum];
	memset(m_pulArrayThreadCalled, 0x00, sizeof(unsigned long) * m_iQueryThreadNum);
	m_iOpLogFrom = 0;
	int iLoop;
	for(iLoop = 0; iLoop < SAVE_OPERATOR_LOG; iLoop++) {
		m_strOpLog[iLoop] = "";
	}

}

string CTipManage::LookSysRunInfo()
{
	string strResult = "";
	int iNum;
	char szStr[256];
	map<const string, CSpecialOperator>::iterator iIter;
	map<const string, CTMInfo *>::iterator iIterInfo;
	pthread_mutex_lock(&m_pmtMutex);
	strResult += "\"operator\":{";
	iNum = m_mapSOList.size();
	snprintf(szStr, sizeof(szStr), "\"number_%d\":[", iNum);
	strResult += szStr;
	int iLoop;
	for(iLoop = 1, iIter = m_mapSOList.begin(); iIter != m_mapSOList.end(); iIter++, iLoop++) {
		CSpecialOperator & t = iIter->second;
		snprintf(szStr, sizeof(szStr), "{\"id\":\"%d\",\"key\":\"%s\",\"value\":\"%s\",\"state\":\"%d\"}", iLoop, t.strKey.c_str(), t.strValue.c_str(), t.iState);
		strResult += szStr;
	}
	printf("first run-------------------------------------------------------->>>>>\n");
	strResult += "]},\"info\":{"; //����operator����ʼinfo
	iNum = m_mapInfo.size();
	snprintf(szStr, sizeof(szStr), "\"number\":\"%d\",\"detail\":[", iNum);
	strResult += szStr;
	for(iLoop = 1, iIterInfo = m_mapInfo.begin(); iIterInfo != m_mapInfo.end(); iIterInfo++, iLoop++) {
		CTMInfo * t = iIterInfo->second;
		snprintf(szStr, sizeof(szStr), "{\"id\":\"%d\",\"taskname\":\"%s\",\"useidx\":\"%d\",\"pNULL\":\"%d:%d\",\"hit\":\"%lu\"}", iLoop, iIterInfo->first.c_str(), t->m_iCurrentUseTreeIdx, 
				t->m_cTreeArray[0] ? 1:0, t->m_cTreeArray[1] ? 1:0, t->m_ulHitTime);
		strResult += szStr;
	}
	strResult += "]"; //over detail
	strResult += "}"; //����info
	pthread_mutex_unlock(&m_pmtMutex);
	return strResult;
}

//�������������sleep����
//���ﵥ�����m_mapInfoʹ������,����ȡ���map���ݵ�ʱ��,û�м���,���ܻ�������,�������Ҫ����
//1. ���û����������˳�
//2. ��Զ����map��ȡ���ĵ�һ��������û��ʱ��˳��,������������
//3. �����һ��״̬����δ����(���ܴ����л��߽���),�ȴ���һ��
//4. ����Ǹ��»���ɾ��,����Ƿ��Ѿ����ڵ�ǰ����,û�еĻ�,����[�������һ���Ѿ����ڵ�]
//5. �����ɾ�����߸���,ȡ��CTMInfo
//6. �����ɾ��,����Ҫ�ȴ�һ��,��ֹ��ǰ�в�ѯ��������
//7. ���£� �����쳣���,���ż���һ���µ�CTreeArray,Ȼ��ѵ�ǰʹ�õ�CTreeArray�±껻���µ�,���ȴ�һ����ʱ��ѯ��ʱ��,�Ѿɵĸ�ɾ����
void CTipManage::UpdateOperator()
{
	do {
		map<const string, CSpecialOperator>::iterator iIter;
		map<const string, CTMInfo *>::iterator iIter2;
		string strTaskName;
		CTMInfo  * pTMInfo;
		int iState;
		//*************************************ʶ��״̬  begin
		pthread_mutex_lock(&m_pmtMutex);
		if(m_mapSOList.size() == 0) { //��ǰû�в���
			pthread_mutex_unlock(&m_pmtMutex);
			break;
		}
		iIter = m_mapSOList.begin(); //�����һ������
		string strValue = iIter->second.strValue;
		strTaskName = iIter->second.strKey;
		iState = iIter->second.iState;
		if(iState == OPERATOR_TYPE_DO_1) {
			iIter->second.iState = OPERATOR_TYPE_DO_2;
		} else {
			pthread_mutex_unlock(&m_pmtMutex);
			break;
		}
		if(strValue == "u" || strValue == "d") { //��ӻ���ɾ��,����ȡ����
			iIter2 = m_mapInfo.find(strTaskName);
			if((iIter2 == m_mapInfo.end() && (strValue == "u" || strValue == "d")) || (iIter2 != m_mapInfo.end() && strValue == "a")) { //û���ҵ�
				printf("error: can't find the task info[%s %d] or add existed task\n", __FILE__, __LINE__);
				iIter->second.iState = OPERATOR_TYPE_DO_3; //�������Ǵ���
				pthread_mutex_unlock(&m_pmtMutex);
				break;
			}
			if(iIter2 != m_mapInfo.end()) {
				pTMInfo = iIter2->second;
				if(strValue == "d") { //�����ɾ��,��ô����Ҫɾ��������
					m_mapInfo.erase(iIter2);
				}
			}
		}
		pthread_mutex_unlock(&m_pmtMutex);
		bool bChange = false;
		//*************************************ʶ��״̬  over
		if(strValue == "a") { //���
			CTMInfo * pTmp = new CTMInfo;
			if(!pTmp) {
				printf("error::new memory error[%s %d]\n", __FILE__, __LINE__);
				break;
			}
			pTmp->m_cTreeArray[0] = new CTreeArray(strTaskName);
			if(!(pTmp->m_cTreeArray[0])) {
				printf("error::new array tree failed[%s %d]\n", __FILE__, __LINE__);
				delete pTmp;
				break;
			}
			pTmp->m_iCurrentUseTreeIdx = 0;
			pthread_mutex_lock(&m_pmtMutex);
			if(m_mapInfo.find(strTaskName) != m_mapInfo.end()) {
				bChange = true;
			} else {
				if(pTmp->m_cTreeArray[0]->GetTaskUseSize() == 0) { //���ص����񲻴��ڡ�
					bChange = true;
				} else {
					m_mapInfo[strTaskName] = pTmp;
				}
			}
			pthread_mutex_unlock(&m_pmtMutex);
			if(bChange) {
				printf("maybe something wrong, hard happen (fast put %s %d)\n", __FILE__, __LINE__);
				SAFE_DELETE_SIG(pTmp->m_cTreeArray[0]);
				SAFE_DELETE_SIG(pTmp);
				break;
			}
		} else if(strValue == "d") { //ɾ�� 
			sleep(ONE_SEACH_MAXTIME);
			SAFE_DELETE_SIG(pTMInfo->m_cTreeArray[0]);
			SAFE_DELETE_SIG(pTMInfo->m_cTreeArray[1]);
			SAFE_DELETE_SIG(pTMInfo);
		} else { //strValue == "u" //����
			int iCurUse = pTMInfo->m_iCurrentUseTreeIdx;
			int iNextUse = 1 - iCurUse;
			if(iCurUse != 0 && iCurUse != 1) {
				printf("error::current use tree idx is :%d[%s %d]\n", iCurUse, __FILE__, __LINE__);
				break;
			}
			if(pTMInfo->m_cTreeArray[iNextUse]) { //�쳣�ж�,����Ӧ�ò������
				sleep(ONE_SEACH_MAXTIME);
				SAFE_DELETE_SIG(pTMInfo->m_cTreeArray[iNextUse]);
				printf("error: i don't know why the array is not null,but i still delete it again\n");
			}
			pTMInfo->m_cTreeArray[iNextUse] = new CTreeArray(strTaskName);
			if(!pTMInfo->m_cTreeArray[iNextUse]) {
				printf("error::new array tree failed[%s %d]\n", __FILE__, __LINE__);
				break;
			}
			pTMInfo->m_iCurrentUseTreeIdx = iNextUse;
			sleep(ONE_SEACH_MAXTIME);
			SAFE_DELETE_SIG(pTMInfo->m_cTreeArray[iCurUse]);
		}
		//����ò���
		pthread_mutex_lock(&m_pmtMutex);
		if((iIter = m_mapSOList.find(strTaskName)) == m_mapSOList.end()) { //�����һ������
			printf("error::when execute the task(%s), the task disappear[%s %d]\n", strTaskName.c_str(), __FILE__, __LINE__);
			pthread_mutex_unlock(&m_pmtMutex);
			break;
		} else {
			iIter->second.iState = OPERATOR_TYPE_DO_3;
			m_mapSOList.erase(iIter);
		}
		pthread_mutex_unlock(&m_pmtMutex);
	} while(0);
}
unsigned int CTipManage::GetValueNum(map<const string, string> &mapKeyValue, const string &strKey, const unsigned int uiLow, const unsigned int uiUpper, const unsigned int uiDefault)
{
	if(uiDefault > uiUpper || uiDefault < uiLow || uiUpper < uiLow) {
		return (uiUpper + uiLow) / 2;
	}

	string strValue = GetValueStr(mapKeyValue, strKey);
	if(strValue == "") return uiDefault;
	unsigned int uiValue = atoi(strValue.c_str());
	if(uiValue < uiLow) return uiLow;
	if(uiValue > uiUpper) return uiUpper;
	return uiValue;
}

string CTipManage::GetValueStr(map<const string, string> &mapKeyValue, const string &strKey)
{
	map<const string, string>::iterator iIter;
	iIter = mapKeyValue.find(strKey);
	if(iIter != mapKeyValue.end()) {
		return iIter->second;
	}
	return "";
}

string CTipManage::SearchTip(map<const string, string> & mapKeyValue)
{
	string strResult = "";
	PRINT_T("before treeArraymng");
	//����ڶ�������Ҫ��ֵ
	//Ϊ���ܹ��ڲ��������������������
	if(!CTreeArrayMng::GetInstance().SearchResult(mapKeyValue, m_mapInfo, strResult)) {
		strResult = "\"detailall\":[]";
	}
	PRINT_T("after treeArraymng");
	return strResult;
}

string CTipManage::SearchDebug(map<const string, string> & mapKeyValue)
{
	string strResult    = "";
	string strTaskName  = GetValueStr(mapKeyValue, "x"); //��������
	string strPrefix    = GetValueStr(mapKeyValue, "p"); //��ѯ��ǰ׺
	string strSearchKey = GetValueStr(mapKeyValue, "w"); //��ѯ��
	unsigned int uiDepth    = GetValueNum(mapKeyValue, "d", 1, 30, 5); //��ʾ�����
	unsigned int uiTreeType = GetValueNum(mapKeyValue, "t", 0, 31,  0); //��ʾ����������


	map<const string, CTMInfo *>::iterator iIterTM;
	iIterTM = m_mapInfo.find(strTaskName);
	if(iIterTM == m_mapInfo.end()) {
		return "debug:'no taskname',";
	}

	if(iIterTM->second == NULL) {
		return "debug:'no bad task',";
	}
	CTreeArray * pTreeArray = iIterTM->second->GetTreeArray();
	return pTreeArray->PrintTree(strPrefix, strSearchKey, uiDepth, uiTreeType);
}

string CTipManage::SpecialXuan(int iThreadNum)
{
	string strRes = "\"is\":{";
	char szInfo[128];
	sprintf(szInfo, "\"threadId\":\"%d\",", iThreadNum);
	strRes += szInfo;
	int iLoop;
	strRes += "\"threadHit\":{\n";
	for(iLoop = 0; iLoop < m_iQueryThreadNum; iLoop++) {
		sprintf(szInfo, "\"thread_%d\":\"%lu\",", iLoop, m_pulArrayThreadCalled[iLoop]);
		strRes += szInfo;
	}
	strRes[strRes.size() - 1] = '}'; //�����һ�����Ż���������
	strRes += ",\n";

	strRes += "\"taskList\":[\n";
	map<const string, CTMInfo *>::iterator iIter;
	for(iIter = m_mapInfo.begin(); iIter != m_mapInfo.end(); iIter++) {
		CTMInfo * pTmp = iIter->second;
		CTreeArray * pTree = pTmp->m_cTreeArray[pTmp->m_iCurrentUseTreeIdx];
		strRes += "{\"name\":\"" + iIter->first + "\"";
		sprintf(szInfo, ",\n\"hit\":\"%lu\",\"idx\":\"%d\"", pTmp->m_ulHitTime, pTmp->m_iCurrentUseTreeIdx); 
		strRes += szInfo;
		if(pTree) {
			unsigned int uiSize = pTree->GetTaskUseSize();
			float fSize = uiSize;
			char cTail = 'B';
			if(uiSize > 1024 * 1024 * 1024) {
				cTail = 'G';
				fSize = ((float)(uiSize)) / 1024 / 1024 / 1024;
			} else if(uiSize > 1024 * 1024) {
				cTail = 'M';
				fSize = ((float)uiSize) / 1024 / 1024;
			} else if(uiSize > 1024) {
				cTail = 'K';
				fSize = ((float)uiSize) / 1024;
			}
			sprintf(szInfo, ",\"size\":\"%.2f%c\"", fSize, cTail);
			strRes += szInfo;
		}
		strRes += "},";
	}
	strRes[strRes.size() - 1] = ']'; //�����һ�����Ż���������
	strRes += ",\n";
	if(m_iOpLogFrom != 0 || m_strOpLog[SAVE_OPERATOR_LOG - 1] != "") {
		strRes += "\"operator_log\":{\n";
		iLoop = (m_iOpLogFrom - 1 + SAVE_OPERATOR_LOG) % SAVE_OPERATOR_LOG;
		int iId = 0;
		for(; iLoop != m_iOpLogFrom && m_strOpLog[iLoop] != ""; iLoop=((iLoop-1+SAVE_OPERATOR_LOG)%SAVE_OPERATOR_LOG)) {
			iId++;
			sprintf(szInfo, "\"log_%d\":\"", iId);
			strRes += szInfo + m_strOpLog[iLoop] + "\",";
		}
		if(iLoop == m_iOpLogFrom && m_strOpLog[iLoop] != "") {
			iId++;
			sprintf(szInfo, "\"log_%d\":\"", iId);
			strRes += szInfo + m_strOpLog[iLoop] + "\"";
			strRes += "}";
		} else {
			strRes[strRes.size() - 1] = '}'; //�����һ�����Ż���������
		}
		strRes += "\n";
	}
	strRes += "}"; //is over
	return strRes;
}
string CTipManage::SpecialOperator(map<const string, string> & mapKeyValue)
{
	string strResult = "\"operator\":{";
	string strKey   = GetValueStr(mapKeyValue, "k"); //��������
	string strTaskName  = GetValueStr(mapKeyValue, "v"); //��������
	printf("key:%s; taskname:%s\n", strKey.c_str(), strTaskName.c_str());
	CSpecialOperator  cOperator;
	time_t timep;
	struct tm *pTM;
	char szTime[128];
	time(&timep); /*��ȡtime_t���͵ĵ�ǰʱ��*/
	pTM = localtime(&timep);
	//pTM = gmtime(&timep); /*ת��Ϊstruct tm�ṹ��UTCʱ��*/
	sprintf(szTime, "%d-%d-%d %d/%d/%d->", 1900 + pTM->tm_year, 1 + pTM->tm_mon, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec);

	if(strKey == "a" || strKey == "d" || strKey == "u") {
		cOperator.strKey = strTaskName;
		cOperator.strValue = strKey;
		cOperator.iState  = OPERATOR_TYPE_DO_1;
		bool bHave = false;
		string strInfo;
		pthread_mutex_lock(&m_pmtMutex);
		if(m_mapInfo.find(strTaskName) != m_mapInfo.end()) {
			bHave = true;
		}
		if(!bHave && strKey == "d" || !bHave && strKey == "u" || bHave && strKey == "a") {
			strInfo = "bad operator";
		} else if(m_mapSOList.find(strTaskName) != m_mapSOList.end()) { //�Ѿ�����
			strInfo = "already one deal to this task, please wait...";
		} else {
			m_mapSOList[strTaskName] = cOperator;
			strInfo = "has add to dealing list";
		}
		string strLog = szTime + strKey + "->" + strTaskName + "->" + strInfo;
		m_strOpLog[m_iOpLogFrom] = strLog;
		m_iOpLogFrom = (m_iOpLogFrom + 1) % SAVE_OPERATOR_LOG;
		pthread_mutex_unlock(&m_pmtMutex);
		strResult += "\"info\":\"" + strInfo + "\"";
	} else if(strKey == "l") { //�鿴
		strResult += LookSysRunInfo();
	} else {
		strResult += "\"info\":\"unknown operator\"";
	}
	strResult += "}"; //</operator>";
	printf("operator----->>>%s\n", strResult.c_str());
	return strResult;
}

string CTipManage::Search(map<const string, string> & mapKeyValue, int iThreadId) 
{
	string strResult = "";
	map<const string, string>::iterator iIter;
	char szTime[64];
	timeval startt;
	gettimeofday(&startt,0);
	long starttime = (startt.tv_sec * 1000000 + startt.tv_usec);
	PRINT_T("begin");
	string strOP = "";
	iIter = mapKeyValue.find("op");
	if(iIter != mapKeyValue.end()) {
		strOP = iIter->second;
	}
	if(strOP == "dc") {
		//strResult += StrUtils::XmlSpecialChar(TIPSERVER_PARA_INTRO);
		strResult += TIPSERVER_PARA_INTRO;
	} else if(strOP == "is") {
		m_pulArrayThreadCalled[iThreadId]++;
		strResult += SearchTip(mapKeyValue);
	} else if(strOP == "dg") {
		strResult += SearchDebug(mapKeyValue);
	} else if(strOP == "so") {
		strResult += SpecialOperator(mapKeyValue);
	} else if(strOP == "xuan") {
		strResult += SpecialXuan(iThreadId);
	} else {
		strResult += "ret:'no result'";
	}
	gettimeofday(&startt,0);
	snprintf(szTime, sizeof(szTime), "\"rettime\":\"%luus\",\n", (startt.tv_sec * 1000000 + startt.tv_usec) - starttime);
	strResult = szTime + strResult;
	printf("total use-->#####################%s\n", szTime);
	//printf("%s\n", strResult.c_str());
	return strResult;
}
