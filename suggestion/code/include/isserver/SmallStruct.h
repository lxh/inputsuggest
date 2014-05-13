#ifndef _SMALL_STRUCT_H_
#define _SMALL_STRUCT_H_
#include "isserver/TreeArray.h"
#include <string>
#include <map>
#include <vector>
using namespace std;

class CTMInfo {
public:
	CTMInfo() {
		m_ulHitTime = 0;
		m_cTreeArray[0] = NULL;
		m_cTreeArray[1] = NULL;
		m_iCurrentUseTreeIdx = -1;
	};
	CTreeArray * GetTreeArray() {
		if(m_iCurrentUseTreeIdx < 0 || m_iCurrentUseTreeIdx > 1) {
			return NULL;
		}
		return m_cTreeArray[m_iCurrentUseTreeIdx];
	};
	int       m_iCurrentUseTreeIdx; //��ǰ����ʹ�õ������±�
	CTreeArray * m_cTreeArray[2];         //���������СΪ2��Ϊ�����ݸ���ʹ��
	unsigned long m_ulHitTime; //�����ر�׼
};
class CSpecialOperator {
public:
	string strKey;   //�����Ĺؼ���
	string strValue; //������ֵ
	int iState; //����״̬
};







#endif
