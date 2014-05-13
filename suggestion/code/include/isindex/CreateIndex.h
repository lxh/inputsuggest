#ifndef _CREATE_INDEX_H_
#define _CREATE_INDEX_H_
#include <vector>             
#include <string>             
#include <map>                
#include "util/SysConfigSet.h"
#include "util/WriteBigFile.h"
#include "isindex/WordStringInfo.h"
#include "common/XHTreeStruct.h"
#include "isindex/IndexDataInfo.h"

using namespace std;    
class CCreateIndex {
public:
	int Run(const string & strTaskName, CSysConfigSet *pclsSysConfigSet, int iType);
	~CCreateIndex();

private:
	int Process();
	int Process_BasicInfo();
	void Process_TreeInfo();
private:
	CSysConfigSet * m_pSysConfigSet; //配置文件的信息                 
	string          m_strTaskName;   //任务的名称                     
	int             m_iDumpOnly;     //只是ｄｕｍｐ数据
};

#endif
