#ifndef _INPUT_TIP_H_
#define _INPUT_TIP_H_
#include <string>
#include "isserver/TipManage.h"

using namespace std;

class CInputTip {
public:
    static CInputTip& GetInstance();          //获取句柄

    void UpdateThread();    //更新数据线程
    int LoadData();                           //加载数据

    string SearchTip(const string &strQuery, int iThreadId); //查询结果
private:
	CTipManage m_cTipManage;
};

#endif
