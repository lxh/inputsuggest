#ifndef _INDEX_DELETE_ITEM_H_
#define _INDEX_DELETE_ITEM_H_

#include <vector>             
#include <string>             
#include <map>                
#include "util/SysConfigSet.h"
#include "util/WriteBigFile.h"
#include "util/ReadBigFile.h"
#include "common/XHTreeStruct.h"
#include "util/FileCommon.h"

using namespace std;

class CIndexDeleteItem {
public:
	int Run(const string & strTaskName, CSysConfigSet *pclsSysConfigSet);
private:
	int Process();
	bool Process_WriteNodeData(const string & strFileName, vector<string> & vecDel, char * & pDictND);
	bool Process_GetDelMap(const string & strFileName, vector<string> & vecDel);

private:
	string m_strTaskName;
	CSysConfigSet *m_pSysConfigSet;
};


#endif
