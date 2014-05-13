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

//����������,�����,���ڴ���,�Ѿ��������
#define OPERATOR_TYPE_DO_1 1
#define OPERATOR_TYPE_DO_2 2
#define OPERATOR_TYPE_DO_3 3

//����������ʾ����Ϣ
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
	string SpecialXuan(int iThreadNum); //���������Ϣ-->����д���ĵ�����

	string LookSysRunInfo();
	string GetValueStr(map<const string, string> &mapKeyValue, const string &strKey);
	unsigned int GetValueNum(map<const string, string> &mapKeyValue, const string &strKey, const unsigned int uiLow, const unsigned int uiUpper, const unsigned int uiDefault);
private:
	map<const string, CTMInfo *> m_mapInfo; //��ѯ����Ӧ����
	map<const string, CSpecialOperator>  m_mapSOList; //һ������ͬʱֻ����һ���������
	pthread_mutex_t m_pmtMutex; //�������������

	int m_iQueryThreadNum;
	unsigned long *m_pulArrayThreadCalled;

	int    m_iOpLogFrom; //��һ������ʹ�õ�
	string m_strOpLog[SAVE_OPERATOR_LOG];
};


#endif
