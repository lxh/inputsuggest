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
	int       m_iCurrentUseTreeIdx; //当前正在使用的树的下标
	CTreeArray * m_cTreeArray[2];         //设置数组大小为2，为了数据更新使用
	unsigned long m_ulHitTime; //不是特别准
};
class CSpecialOperator {
public:
	string strKey;   //操作的关键字
	string strValue; //操作的值
	int iState; //操作状态
};







#endif
