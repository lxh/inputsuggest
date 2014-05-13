#ifndef _DUMP_PICK_INDEX_H_
#define _DUMP_PICK_INDEX_H_
#include "isindex/IndexDataInfo.h"
#include "common/SysCommon.h"

typedef unsigned int U32;

/*
typedef struct _IndexFiles {
	U32 uiTotalSize;
	//字符串信息
	U32 uiChineseString;
	U32 uiSimpleString;
	U32 uiSpellString;

	//扩展信息
	U32 uiChineseExtra;
	U32 uiSpellExtra;
	U32 uiSimpleExtra;
	U32 uiCenterChineseExtra;
	U32 uiCenterSpellExtra;
	U32 uiCenterSimpleExtra;

	//树节点信息
	U32 uiChineseTreeNode;
	U32 uiSpellTreeNode;
	U32 uiSimpleTreeNode;
	U32 uiCenterChineseTreeNode;
	U32 uiCenterSpellTreeNode;
	U32 uiCenterSimpleTreeNode;
	U32 uiExChineseTreeNode;
	U32 uiExSpellTreeNode;
	U32 uiExSimpleTreeNode;
	U32 uiExCenterChineseTreeNode;
	U32 uiExCenterSpellTreeNode;
	U32 uiExCenterSimpleTreeNode;

	//树节点的数据部分内容　
	U32 uiChineseNodeData;
	U32 uiSpellNodeData;
	U32 uiSimpleNodeData;
	U32 uiCenterChineseNodeData;
	U32 uiCenterSpellNodeData;
	U32 uiCenterSimpleNodeData;

	//混合搜索使用
	U32 uiSpellMixSearch;
	U32 uiSimpleMixSearch;
	U32 uiCenterSpellMixSearch;
	U32 uiCenterSimpleMixSearch;
	U32 uiExSpellMixSearch;
	U32 uiExSimpleMixSearch;
	U32 uiExCenterSpellMixSearch;
	U32 uiExCenterSimpleMixSearch;
}IndexFiles;
*/
typedef struct _IndexFiles {
	U32 uiSize;
	U32 bSelect[DICT_MAX_SIZE];
	U32 uiString[DICT_MAX_SIZE];
	U32 uiExtra[DICT_MAX_SIZE];
	U32 uiTreeNode[DICT_MAX_SIZE];
	U32 uiNodeData[DICT_MAX_SIZE];
	U32 uiMixSearch[DICT_MAX_SIZE];
}IndexFiles;


class CDumpPickIndex {
public:
	static bool Dump(const string & strTaskName, CSysConfigSet *const pSysConfigSet);
	static char * Pick(const string & strTaskName);
private:
	static void WriteData(int & iFd, const char * pData, const int iDataSize);
	static void WriteFile(const string & strFileName, int & iFd, U32 & uiSize);
	static bool Run(const string & strTaskName, CSysConfigSet * const pSysConfigSet);
	static void WriteNodeData(const int iSelect1, const int iSelect2, int & iFd, U32 &uiSize, CSysConfigSet *const pSysConfigSet, IndexFiles &stIndexFiles, const bool bMustAdd, const string &strPath);
};

#endif
