//注： 查询的时候，很少有越界检查，因为检查需要消耗时间，因此正确性交给建立索引的时候去控制
#ifndef _XH_TREE_H_
#define _XH_TREE_H_
#include "common/XHTreeStruct.h"
#include "common/SysCommon.h"
#include <string>

using namespace std;

//#define TREE_TYPE_CHINESE       0
//#define TREE_TYPE_SPELL         1
//#define TREE_TYPE_CENTERCHINESE 2
//#define TREE_TYPE_CENTERSPELL   3

class CTreeBase {
public:
	CTreeBase();
	~CTreeBase();
	const int GetTreeType();
	const int GetTreeMemSize();
	const XHRbNode * GetTreeRoot();
	const XHRbNode * GetTreeLeft(const XHRbNode * p);
	const XHRbNode * GetTreeMid(const XHRbNode * p);
	const XHRbNode * GetTreeRight(const XHRbNode * p);
	const char * GetSSValue_Spell(const  XHRbNode * p, const string & strSimple);
	const char * GetSSValue_Simple(const  XHRbNode * p, const string & strSimple);
	const void * GetTreeWife(const XHRbNode * p);
	const char * GetString(const XHRbNode *p);
	const char * GetNodeString(const XHRbNode *p);
	const void * GetNodeExtraInfo(const XHRbNode *pNode) {};
	const void * GetNodeDataFromId(unsigned int uiId);

public:
	int m_iTreeType;
	//int m_iWifeDataSize;

	char * m_pzExtraInfo;   //扩展信息，结构同上  ---->没有\0
	char * m_pcStringData;  //字符串内容的存储地址,结构同上
	//下面两个对于汉字树与拼音树对应的结构和信息不同
	//m_pzExtraInfo，汉字对应的为：用户扩展信息； 拼音：对的上级信息
	//对应-----汉字结构
	//数据的格式有两种，分别为包含扩展信息和不包含扩展信息
	//1. 不包含: p1   p2
	//           |    |
	//          \|/  \|/
	//           key\0key2\0
	//2. 包含:  p1        p2
	//          |         |
	//         \|/       \|/
	//     4字节key\04字节key2\0
	//注：4字节为后面的key对应的数据扩展部分在m_pzExtraInfo的位置
	//如果4字节为0，代表没有扩展信息
	char * m_pzTreeWife;    //树的数据信息,结构同上 --->没有\0
	char * m_pstTreeRoot;   //XHRbNode, 结构为：前四个字节为大小，后面为内容
	char * m_pstSimpleSelf; //拼音树(包含中间拼音树),简拼指向全拼的内容uiSpellSelf部分

	//获取在制定内存移动一段距离后的位置
	const char * GetPMove(const char * p, const unsigned int iMove);
};

class CTreeChinese : public CTreeBase {
public:
	CTreeChinese();
	const void * GetNodeExtraInfo(const XHRbNode *pNode);
	const char * GetNodeExtraStringFromData(const XHChineseData* pData);
	const void * GetNodeStringFromData(const XHChineseData* pData);
	const char * GetNodeOrigStringFromData(const XHChineseData* pData);
	const unsigned int GetIdFromNode(const XHRbNode *pNode);
};

class CTreeSpell: public CTreeBase {
public:
	CTreeSpell();
	const void * GetNodeExtraInfo(const XHRbNode *pNode);
	const unsigned int GetNodeExtraNums(const XHRbNode *pNode);
	const void * GetNodeString(const XHChineseData* pData);
};
class CTreeSimple: public CTreeSpell {
public:
	CTreeSimple();
};

class CTreeCenterChinese: public CTreeSpell {
public:
	CTreeCenterChinese();
	const void * GetNodeExtraInfo(const XHSpellData *pData);
	const unsigned int GetNodeExtraNums(const XHSpellData *pData);
};

class CTreeCenterSpell: public CTreeSpell {
public:
	CTreeCenterSpell();
};

class CTreeCenterSimple: public CTreeSpell {
public:
	CTreeCenterSimple();
};

class CTreeExChinese: public CTreeSpell {
public:
	CTreeExChinese();
};

class CTreeExSpell: public CTreeSpell {
public:
	CTreeExSpell();
};

class CTreeExSimple: public CTreeSpell {
public:
	CTreeExSimple();
};

class CTreeExCenterChinese: public CTreeSpell {
public:
	CTreeExCenterChinese();
};

class CTreeExCenterSpell: public CTreeSpell {
public:
	CTreeExCenterSpell();
};

class CTreeExCenterSimple: public CTreeSpell {
public:
	CTreeExCenterSimple();
};
/*
class CTreeDataManage {
};
*/

#endif
