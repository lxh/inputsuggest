#ifndef _INPUT_TIP_H_
#define _INPUT_TIP_H_
#include <string>
#include "isserver/TipManage.h"

using namespace std;

class CInputTip {
public:
    static CInputTip& GetInstance();          //��ȡ���

    void UpdateThread();    //���������߳�
    int LoadData();                           //��������

    string SearchTip(const string &strQuery, int iThreadId); //��ѯ���
private:
	CTipManage m_cTipManage;
};

#endif
