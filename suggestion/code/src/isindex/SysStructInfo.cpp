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
	printf("文件名: %s\n\n", m_szName);
	printf("汉字数据量: %d\n", m_uiChineseDataNum);
	printf("扩展汉字数据量: %d\n", m_uiExChineseDataNum);
	printf("拼音数据量: %d\n", m_uiSpellDataNum);
	printf("扩展拼音数据量: %d\n", m_uiExSpellDataNum);
	printf("简拼数据量: %d\n", m_uiSimpleDataNum);
	printf("扩展简拼数据量: %d\n", m_uiExSimpleDataNum);
	printf("中间汉字数据量: %d\n", m_uiCenterChineseDataNum);
	printf("扩展中间汉字数据量: %d\n", m_uiExCenterChineseDataNum);
	printf("中间拼音数据量: %d\n", m_uiCenterSpellDataNum);
	printf("扩展中间拼音数据量: %d\n", m_uiExCenterSpellDataNum);
	printf("中间简拼数据量:%d\n", m_uiCenterSimpleDataNum);
	printf("扩展中间简拼数据量: %d\n", m_uiExCenterSimpleDataNum);
	printf("\n");
	printf("扩展拼音含有多个结果数据的量: %d\n", m_uiExSpellAddNum);
	printf("扩展简拼含有多个结果数据的量: %d\n", m_uiExSimpleAddNum);
	printf("扩展中间汉字含有多个结果数据的量: %d\n", m_uiExCenterChineseAddNum);
	printf("扩展中间拼音含有多个结果数据的量: %d\n", m_uiExCenterSpellAddNum);
	printf("扩展中间简拼含有多个结果数据的量: %d\n", m_uiExCenterSimpleAddNum);
	printf("describe-------------------------------------------->>>end\n");
}
