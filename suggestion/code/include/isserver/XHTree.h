//ע�� ��ѯ��ʱ�򣬺�����Խ���飬��Ϊ�����Ҫ����ʱ�䣬�����ȷ�Խ�������������ʱ��ȥ����
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

	char * m_pzExtraInfo;   //��չ��Ϣ���ṹͬ��  ---->û��\0
	char * m_pcStringData;  //�ַ������ݵĴ洢��ַ,�ṹͬ��
	//�����������ں�������ƴ������Ӧ�Ľṹ����Ϣ��ͬ
	//m_pzExtraInfo�����ֶ�Ӧ��Ϊ���û���չ��Ϣ�� ƴ�����Ե��ϼ���Ϣ
	//��Ӧ-----���ֽṹ
	//���ݵĸ�ʽ�����֣��ֱ�Ϊ������չ��Ϣ�Ͳ�������չ��Ϣ
	//1. ������: p1   p2
	//           |    |
	//          \|/  \|/
	//           key\0key2\0
	//2. ����:  p1        p2
	//          |         |
	//         \|/       \|/
	//     4�ֽ�key\04�ֽ�key2\0
	//ע��4�ֽ�Ϊ�����key��Ӧ��������չ������m_pzExtraInfo��λ��
	//���4�ֽ�Ϊ0������û����չ��Ϣ
	char * m_pzTreeWife;    //����������Ϣ,�ṹͬ�� --->û��\0
	char * m_pstTreeRoot;   //XHRbNode, �ṹΪ��ǰ�ĸ��ֽ�Ϊ��С������Ϊ����
	char * m_pstSimpleSelf; //ƴ����(�����м�ƴ����),��ƴָ��ȫƴ������uiSpellSelf����

	//��ȡ���ƶ��ڴ��ƶ�һ�ξ�����λ��
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
