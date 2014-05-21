#include "isindex/IndexTreeInfo.h"
#include "util/FileCommon.h"
#include "util/ReadBigFile.h"
#include "util/XHStrUtils.h"
#include "isindex/CreateIndex.h"
#include "util/WriteLog.h"
#include "util/CompressPinyin.h"
#include "util/DumpPickIndex.h"
#include "isindex/IndexDeleteItem.h"

int CCreateIndex::Run(const string & strTaskName, CSysConfigSet *pclsSysConfigSet, int iType)
{
	m_strTaskName = strTaskName;
	m_pSysConfigSet = pclsSysConfigSet;
	m_iDumpOnly = iType;
	string strPath = m_pSysConfigSet->GetString(PARA_CONFIG_pathdata) + "/" + m_strTaskName + "/";
	string strLog = strPath + m_pSysConfigSet->GetString(PARA_CONFIG_tipindexlog);
	if(m_pSysConfigSet->GetInt(PARA_CONFIG_useindexlog)) {
		CWriteLog::GetInstance(strLog);
	}

	CCompressPinyin::GetInstance();
	Process();
	return 0;
}

CCreateIndex::~CCreateIndex()
{
	CWriteLog::GetInstance().CloseFile();
}

//���������Ϣ������Ҫʹ�õ���һЩ��Ϣ��
int CCreateIndex::Process_BasicInfo()
{
	CIndexDataInfo cIDI;
	cIDI.Run(m_strTaskName, m_pSysConfigSet);
	return 0;
}

void CCreateIndex::Process_TreeInfo()
{
	CIndexTreeInfo cITI;
	cITI.Run(m_strTaskName, m_pSysConfigSet);
}

int CCreateIndex::Process_DeleteItem()
{
	CIndexDeleteItem cIDI;
	return cIDI.Run(m_strTaskName, m_pSysConfigSet);
}

//��������
int CCreateIndex::Process()
{
	switch(m_iDumpOnly) {
		case 1: //basic
			Process_BasicInfo();
			break;
		case 2: //treeinfo
			Process_TreeInfo();
			break;
		case 3: //treeinfo
			CDumpPickIndex::Dump(m_strTaskName, m_pSysConfigSet); 
			break;
		case 4: //delete some item
			if(Process_DeleteItem()) {
				CDumpPickIndex::Dump(m_strTaskName, m_pSysConfigSet); 
			}
			break;
		default: //case 0:
			Process_BasicInfo();
			Process_TreeInfo();
			CDumpPickIndex::Dump(m_strTaskName, m_pSysConfigSet); 
			break;
	}
	/*
	if(!m_iDumpOnly) {
		Process_BasicInfo();
		Process_TreeInfo();
	}
	CDumpPickIndex::Dump(m_strTaskName, m_pSysConfigSet); 
	*/
	return 0;
}
