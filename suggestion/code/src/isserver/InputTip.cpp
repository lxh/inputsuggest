//��Ҫ�������ս���ķ�װ��������ʾʵ��Ĺ���ȹ���
#include <stdio.h>
#include "isserver/UserParamterCommon.h"
#include "isserver/InputTip.h"
#include "util/XHStrUtils.h"
#include "util/SysConfigSet.h"

using namespace std;

CInputTip &CInputTip::GetInstance()
{
    static CInputTip cit;
    return cit;
}

void CInputTip::UpdateThread()
{
	string strTaskList = G_STR(tasklist);
	vector<string> vecTaskList;
	XHStrUtils::StrTokenize(vecTaskList, strTaskList, ";", true);

	int iLoop;
	for(iLoop = 0; iLoop < vecTaskList.size(); iLoop++) {
		map<const string, string> mapInit;
		mapInit["op"] = "so";               //�������
		mapInit["k"] = "a";                //���
		mapInit["v"] = vecTaskList[iLoop]; //��������
		m_cTipManage.Search(mapInit, -1);
	}

    do {
		for(iLoop = 0; iLoop < 60; iLoop+=1) {
			sleep(1);
			m_cTipManage.UpdateOperator();
		}
        LoadData();
    } while(1);
}

//�����������ļ����ݷ����ı�,�Զ���������ʹ��,��ʱֻ�ṩurl����
int CInputTip::LoadData()
{
    return 0;
}

string CInputTip::SearchTip(const string &strQuery, int iThreadId)
{
	int iLoop;
	vector<string> vecSplit;
	map<const string, string> mapPara;
	XHStrUtils::StrTokenize(vecSplit, strQuery, "&", true);
	for(iLoop = 0; iLoop < vecSplit.size(); iLoop++) {
		vector<string> vecTerms;
		
		XHStrUtils::StrTokenize(vecTerms, vecSplit[iLoop], "=");
		if(vecTerms.size() == 2) {
			if(vecTerms[1].size() != 0) {
				mapPara[vecTerms[0]] = vecTerms[1];
			}
		}
	}
	printf("%s\n", strQuery.c_str());
	return m_cTipManage.Search(mapPara, iThreadId);
}
