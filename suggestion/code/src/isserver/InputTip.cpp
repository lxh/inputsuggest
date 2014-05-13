//主要负责最终结果的封装、输入提示实体的管理等工作
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
		mapInit["op"] = "so";               //特殊操作
		mapInit["k"] = "a";                //添加
		mapInit["v"] = vecTaskList[iLoop]; //任务名称
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

//这里是设置文件内容发生改变,自动加载索引使用,暂时只提供url更新
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
