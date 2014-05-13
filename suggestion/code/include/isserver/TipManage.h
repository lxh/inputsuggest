#ifndef _TIP_MANAGE_H_
#define _TIP_MANAGE_H_
#include "common/SysCommon.h"
#include <map>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "isserver/SmallStruct.h"
#include "isserver/TreeArrayMng.h"

using namespace std;

#define SAVE_OPERATOR_LOG 20

//操作的类型,刚添加,正在处理,已经处理完成
#define OPERATOR_TYPE_DO_1 1
#define OPERATOR_TYPE_DO_2 2
#define OPERATOR_TYPE_DO_3 3

//管理输入提示的信息
class CTipManage {
public:
public:
	CTipManage();
	void UpdateOperator();
	string Search(map<const string, string> & mapKeyValue, int iThreadId);
private:
	string SearchTip(map<const string, string> & mapKeyValue);
	string SearchDebug(map<const string, string> & mapKeyValue);
	string SpecialOperator(map<const string, string> & mapKeyValue);
	string SpecialXuan(int iThreadNum); //输出环境信息-->不会写入文档里面

	string LookSysRunInfo();
	string GetValueStr(map<const string, string> &mapKeyValue, const string &strKey);
	unsigned int GetValueNum(map<const string, string> &mapKeyValue, const string &strKey, const unsigned int uiLow, const unsigned int uiUpper, const unsigned int uiDefault);
private:
	map<const string, CTMInfo *> m_mapInfo; //查询串对应的树
	map<const string, CSpecialOperator>  m_mapSOList; //一个任务同时只能有一个特殊操作
	pthread_mutex_t m_pmtMutex; //保护特殊操作的

	int m_iQueryThreadNum;
	unsigned long *m_pulArrayThreadCalled;

	int    m_iOpLogFrom; //下一个可以使用的
	string m_strOpLog[SAVE_OPERATOR_LOG];
};


#endif
