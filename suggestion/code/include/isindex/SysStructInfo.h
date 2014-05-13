#ifndef _SYS_STRUCT_INFO_H_
#define _SYS_STRUCT_INFO_H_
#define FILE_NAME_LEN 256

class CSysStructInfo {
public:
	CSysStructInfo(const char * szName);
	void Reset();
	bool Read();
	void Write();
	void Describe();
public:
	char m_szName[FILE_NAME_LEN];
	//各项数据的数据量
	unsigned int m_uiChineseDataNum;
	unsigned int m_uiExChineseDataNum;
	unsigned int m_uiSpellDataNum;
	unsigned int m_uiExSpellDataNum;
	unsigned int m_uiSimpleDataNum;
	unsigned int m_uiExSimpleDataNum;
	unsigned int m_uiCenterChineseDataNum;
	unsigned int m_uiExCenterChineseDataNum;
	unsigned int m_uiCenterSpellDataNum;
	unsigned int m_uiExCenterSpellDataNum;
	unsigned int m_uiCenterSimpleDataNum;
	unsigned int m_uiExCenterSimpleDataNum;

	//扩展增加的数量
	//unsigned int m_uiExChineseAddNum; //由于扩展汉字对应上级为汉字,因此不能压缩
	unsigned int m_uiExSpellAddNum;
	unsigned int m_uiExSimpleAddNum;
	unsigned int m_uiExCenterChineseAddNum;
	unsigned int m_uiExCenterSpellAddNum;
	unsigned int m_uiExCenterSimpleAddNum;
};

#endif
