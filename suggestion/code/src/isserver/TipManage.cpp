#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <iconv.h>
#include "util/XHStrUtils.h"
#include "isserver/TipManage.h"
#include "common/TipServerParaIntro.h"
#include "isserver/TopResult.h"

#define ONE_SEACH_MAXTIME 1 //一个查询最多能用的时间

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
	strResult += "]},\"info\":{"; //结束operator，开始info
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
	strResult += "}"; //结束info
	pthread_mutex_unlock(&m_pmtMutex);
	return strResult;
}

//这个函数里面有sleep操作
//这里单方面对m_mapInfo使用了锁,但是取这个map数据的时候,没有加锁,可能会有问题,后面绝对要避免
//1. 如果没有特殊操作退出
//2. 永远处理map中取出的第一个操作（没有时间顺序,纯粹名称排序）
//3. 如果第一个状态不是未处理(可能处理中或者僵死),等待下一次
//4. 如果是更新或者删除,检查是否已经存在当前任务,没有的话,僵死[或者添加一个已经存在的]
//5. 如果是删除或者更新,取出CTMInfo
//6. 如果是删除,首先要等待一会,防止当前有查询正在其中
//7. 更新： 首先异常检查,接着加载一个新的CTreeArray,然后把当前使用的CTreeArray下标换成新的,最后等待一个超时查询的时间,把旧的给删除掉
void CTipManage::UpdateOperator()
{
	do {
		map<const string, CSpecialOperator>::iterator iIter;
		map<const string, CTMInfo *>::iterator iIter2;
		string strTaskName;
		CTMInfo  * pTMInfo;
		int iState;
		//*************************************识别状态  begin
		pthread_mutex_lock(&m_pmtMutex);
		if(m_mapSOList.size() == 0) { //当前没有操作
			pthread_mutex_unlock(&m_pmtMutex);
			break;
		}
		iIter = m_mapSOList.begin(); //处理第一个操作
		string strValue = iIter->second.strValue;
		strTaskName = iIter->second.strKey;
		iState = iIter->second.iState;
		if(iState == OPERATOR_TYPE_DO_1) {
			iIter->second.iState = OPERATOR_TYPE_DO_2;
		} else {
			pthread_mutex_unlock(&m_pmtMutex);
			break;
		}
		if(strValue == "u" || strValue == "d") { //添加或者删除,首先取下来
			iIter2 = m_mapInfo.find(strTaskName);
			if((iIter2 == m_mapInfo.end() && (strValue == "u" || strValue == "d")) || (iIter2 != m_mapInfo.end() && strValue == "a")) { //没有找到
				printf("error: can't find the task info[%s %d] or add existed task\n", __FILE__, __LINE__);
				iIter->second.iState = OPERATOR_TYPE_DO_3; //基本就是错误
				pthread_mutex_unlock(&m_pmtMutex);
				break;
			}
			if(iIter2 != m_mapInfo.end()) {
				pTMInfo = iIter2->second;
				if(strValue == "d") { //如果是删除,那么就需要删除操作了
					m_mapInfo.erase(iIter2);
				}
			}
		}
		pthread_mutex_unlock(&m_pmtMutex);
		bool bChange = false;
		//*************************************识别状态  over
		if(strValue == "a") { //添加
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
				if(pTmp->m_cTreeArray[0]->GetTaskUseSize() == 0) { //加载的任务不存在　
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
		} else if(strValue == "d") { //删除 
			sleep(ONE_SEACH_MAXTIME);
			SAFE_DELETE_SIG(pTMInfo->m_cTreeArray[0]);
			SAFE_DELETE_SIG(pTMInfo->m_cTreeArray[1]);
			SAFE_DELETE_SIG(pTMInfo);
		} else { //strValue == "u" //更新
			int iCurUse = pTMInfo->m_iCurrentUseTreeIdx;
			int iNextUse = 1 - iCurUse;
			if(iCurUse != 0 && iCurUse != 1) {
				printf("error::current use tree idx is :%d[%s %d]\n", iCurUse, __FILE__, __LINE__);
				break;
			}
			if(pTMInfo->m_cTreeArray[iNextUse]) { //异常判断,这里应该不会产生
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
		//清除该操作
		pthread_mutex_lock(&m_pmtMutex);
		if((iIter = m_mapSOList.find(strTaskName)) == m_mapSOList.end()) { //处理第一个操作
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
	//这里第二个参数要传值
	//为了能够在不加锁的情况下正常运行
	if(!CTreeArrayMng::GetInstance().SearchResult(mapKeyValue, m_mapInfo, strResult)) {
		strResult = "\"detailall\":[]";
	}
	PRINT_T("after treeArraymng");
	return strResult;
}

string CTipManage::SearchDebug(map<const string, string> & mapKeyValue)
{
	string strResult    = "";
	string strTaskName  = GetValueStr(mapKeyValue, "x"); //任务类型
	string strPrefix    = GetValueStr(mapKeyValue, "p"); //查询的前缀
	string strSearchKey = GetValueStr(mapKeyValue, "w"); //查询词
	unsigned int uiDepth    = GetValueNum(mapKeyValue, "d", 1, 30, 5); //显示的深度
	unsigned int uiTreeType = GetValueNum(mapKeyValue, "t", 0, 31,  0); //显示的树的类型


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
	strRes[strRes.size() - 1] = '}'; //把最后一个逗号换成右括号
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
	strRes[strRes.size() - 1] = ']'; //把最后一个逗号换成右括号
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
			strRes[strRes.size() - 1] = '}'; //把最后一个逗号换成右括号
		}
		strRes += "\n";
	}
	strRes += "}"; //is over
	return strRes;
}
string CTipManage::SpecialOperator(map<const string, string> & mapKeyValue)
{
	string strResult = "\"operator\":{";
	string strKey   = GetValueStr(mapKeyValue, "k"); //操作名称
	string strTaskName  = GetValueStr(mapKeyValue, "v"); //操作内容
	printf("key:%s; taskname:%s\n", strKey.c_str(), strTaskName.c_str());
	CSpecialOperator  cOperator;
	time_t timep;
	struct tm *pTM;
	char szTime[128];
	time(&timep); /*获取time_t类型的当前时间*/
	pTM = localtime(&timep);
	//pTM = gmtime(&timep); /*转换为struct tm结构的UTC时间*/
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
		} else if(m_mapSOList.find(strTaskName) != m_mapSOList.end()) { //已经存在
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
	} else if(strKey == "l") { //查看
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
