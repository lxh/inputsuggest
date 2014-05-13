#include <cstring>
#include <stdio.h>
#include "isindex/SysStructInfo.h"

CSysStructInfo::CSysStructInfo(const char * szName)
{
	memset(m_szName, 0x00, sizeof(m_szName));
	memcpy(m_szName, szName, strlen(szName));
	//snprintf(m_szName, sizeof(m_szName), "%s", szName);
	Reset();
}

bool CSysStructInfo::Read()
{
	Reset();
	return true;
}

void CSysStructInfo::Reset()
{
	char szName[FILE_NAME_LEN];
	memcpy(szName, m_szName, sizeof(szName));
	memset(this, 0x00, sizeof(CSysStructInfo));
	memcpy(m_szName, szName, sizeof(szName));
}

void CSysStructInfo::Write()
{
}

void CSysStructInfo::Describe()
{
	printf("describe-------------------------------------------->>>begin\n");
	printf("�ļ���: %s\n\n", m_szName);
	printf("����������: %d\n", m_uiChineseDataNum);
	printf("��չ����������: %d\n", m_uiExChineseDataNum);
	printf("ƴ��������: %d\n", m_uiSpellDataNum);
	printf("��չƴ��������: %d\n", m_uiExSpellDataNum);
	printf("��ƴ������: %d\n", m_uiSimpleDataNum);
	printf("��չ��ƴ������: %d\n", m_uiExSimpleDataNum);
	printf("�м人��������: %d\n", m_uiCenterChineseDataNum);
	printf("��չ�м人��������: %d\n", m_uiExCenterChineseDataNum);
	printf("�м�ƴ��������: %d\n", m_uiCenterSpellDataNum);
	printf("��չ�м�ƴ��������: %d\n", m_uiExCenterSpellDataNum);
	printf("�м��ƴ������:%d\n", m_uiCenterSimpleDataNum);
	printf("��չ�м��ƴ������: %d\n", m_uiExCenterSimpleDataNum);
	printf("\n");
	printf("��չƴ�����ж��������ݵ���: %d\n", m_uiExSpellAddNum);
	printf("��չ��ƴ���ж��������ݵ���: %d\n", m_uiExSimpleAddNum);
	printf("��չ�м人�ֺ��ж��������ݵ���: %d\n", m_uiExCenterChineseAddNum);
	printf("��չ�м�ƴ�����ж��������ݵ���: %d\n", m_uiExCenterSpellAddNum);
	printf("��չ�м��ƴ���ж��������ݵ���: %d\n", m_uiExCenterSimpleAddNum);
	printf("describe-------------------------------------------->>>end\n");
}
