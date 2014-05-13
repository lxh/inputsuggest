#ifndef _INDEX_DATA_INFO_H_
#define _INDEX_DATA_INFO_H_
#include <vector>             
#include <string>             
#include <map>                
#include "util/SysConfigSet.h"
#include "util/WriteBigFile.h"
#include "isindex/WordStringInfo.h"
#include "common/XHTreeStruct.h"
#include "isindex/IndexDataInfo.h"


using namespace std;

typedef struct _IdToId {
	//unsigned int uiFrom;
	unsigned int uiTo;
}IdToId;

class CIndexDataInfo {

public:
	int Run(const string & strTaskName, CSysConfigSet *pclsSysConfigSet);

private:
	void InitParamter();
	int Process();
	void Process_BasicInfo_Chinese();
	void Process_BasicInfo_Spell();
	void Process_BasicInfo_Simple();
	int Deal_WordStringInfo(CWordStringInfo & clsWSI, int iType);
	void CreateOneIndex(CWordStringInfo & clsWSI, const int iSelectId);

	void CreateOneIndex_Chinese1(CWordStringInfo & clsWSI, const int iSelectId, const int & iNodeNum);
	void CreateOneIndex_Chinese2(CWordStringInfo & clsWSI, const int iSelectId, const int & iNodeNum);
	void CreateOneIndex_Chinese3(const int iSelectId, const int & iNodeNum);
	int SelectChangeFromEx(int iSelect);
private:
	CSysConfigSet * m_pSysConfigSet; //�����ļ�����Ϣ                 
	string          m_strTaskName;   //���������                     
	string          m_strDataPath;   //���ݵ�·��                     
	bool            m_bDictNeedCreate[DICT_MAX_SIZE]; //��Ҫ����Ĵʵ�
};



#endif
