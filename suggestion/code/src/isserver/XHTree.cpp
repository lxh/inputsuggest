#include "isserver/XHTree.h"

CTreeBase::CTreeBase()
{
	m_iTreeType = -1; //δ��ʼ��
	m_pstTreeRoot = NULL;
	m_pcStringData = NULL;
	m_pzTreeWife = NULL;
	m_pzExtraInfo = NULL;
}

CTreeBase::~CTreeBase()
{
}

const int CTreeBase::GetTreeType()
{
	return m_iTreeType;
}

const int CTreeBase::GetTreeMemSize()
{
	return ((int*)m_pstTreeRoot)[0];
}
const XHRbNode * CTreeBase::GetTreeRoot()
{
	if(((unsigned int *)m_pstTreeRoot)[0] == 4) { //no this tree
		return NULL;
	}
	return (XHRbNode *)GetPMove(m_pstTreeRoot, 4);
}

const XHRbNode * CTreeBase::GetTreeLeft(const XHRbNode * p)
{
	unsigned int ui = p->UncomposeLeft();
	if(!ui) return 0;
	return (XHRbNode *)GetPMove(m_pstTreeRoot, ui);
}

const XHRbNode * CTreeBase::GetTreeMid(const XHRbNode * p)
{
	unsigned int ui = p->UncomposeMid();
	if(!ui) return 0;
	return (XHRbNode *)GetPMove(m_pstTreeRoot, ui);
}

const XHRbNode * CTreeBase::GetTreeRight(const XHRbNode * p)
{
	unsigned int ui = p->UncomposeRight();
	if(!ui) return 0;
	return (XHRbNode *)GetPMove(m_pstTreeRoot, ui);
}

//��ȡ��ƴ��SS��Ϣ
//��ƴ��SS�Ǿ���ѹ����,�����Ҫ�Ƚ�ѹ
//���ص�ָ��ָ�����,�����ƶ�һ���ֽڲ�������������
const char * CTreeBase::GetSSValue_Simple(const  XHRbNode * p, const string & strSimple)
{
	unsigned int ui = p->UncomposeSpellSelf();
	if(!ui) return NULL;
	unsigned int uiPos = ui & (~(1<<31));
	const char *pTmp = (char *)GetPMove(m_pstSimpleSelf, uiPos);
	unsigned char ucSimpleSize = strSimple.size();
	if(ui & (1 << 31)) { //������
		while(pTmp[0] != (char)ALL_OVER_CHAR) {
			if(pTmp[0] == ucSimpleSize) {
				return pTmp + 1;
			}
			if(pTmp[0] > ucSimpleSize) { //�ǰ��մ�С˳���ŵ�
				return NULL;
			}
			pTmp++;
			while(pTmp[0] != (char)SPLIT_MUL_STR) {
				if(pTmp[0] == (char)ALL_OVER_CHAR) {
					return NULL;
				}
				pTmp++;
			}
		}
		if(pTmp[0] == (char)ALL_OVER_CHAR) {
			return NULL;
		}
	} else {
		if(pTmp[0] >= ucSimpleSize) {
			return pTmp;
		} else {
			return NULL;
		}
	}
}
//��ȡȫƴ��SS��Ϣ
const char * CTreeBase::GetSSValue_Spell(const  XHRbNode * p, const string & strSpell)
{
	unsigned int ui = p->UncomposeSpellSelf();
	if(!ui) return NULL;
	if(ui & (1 << 31)) { //������
		ui &= ~(1<<31);
		unsigned char ucSimpleSize = strSpell.size();
		const char * pTmp = (char *)GetPMove(m_pstSimpleSelf, ui);

		if((unsigned char)pTmp[0] >= ucSimpleSize) {
			return pTmp + 1;
		}
		int iLoop = 1;
		while(1) {
			if(pTmp[iLoop] == 0) {
				return NULL;
			}
			if(pTmp[iLoop] == '~') {
				if((unsigned char)pTmp[iLoop + 1] >= ucSimpleSize) {
					return pTmp + iLoop + 1;
				}
			}
			iLoop++;
		}
	}
	return (char *)GetPMove(m_pstSimpleSelf, ui);
}

//��ȡһ���ڵ���ַ���,û��ȥ���м亢��
const char * CTreeBase::GetString(const XHRbNode *p)
{
	const char * pRet = GetNodeString(p);

	if(pRet) return pRet;
	return GetString(GetTreeMid(p));
}

//��ȡ��ǰ�ڵ��������Ϣ,û�з��ؿ�
const char * CTreeBase::GetNodeString(const XHRbNode *p)
{
	//��Ϊ����,ƴ��,�м��ѯ���ַ����ṹ��λ������ͬ��,���ǵ�4-7���ֽ�
	const XHSpellData *sd = (const XHSpellData *)GetTreeWife(p);
	if(sd == NULL) {
		return NULL;
	}
	return GetPMove(m_pcStringData, sd->uiStringPos);
}

//��ȡ����
const void * CTreeBase::GetTreeWife(const XHRbNode * p)
{
	unsigned int ui = p->UncomposeWife();
	if(!ui) return 0;
	//return (void *)GetPMove(m_pzTreeWife, 4 + ui);
	return (void *)GetPMove(m_pzTreeWife, ui);
}

//��ȡ�ַ�����ַָ��ƫ�Ƶ�λ��
const char * CTreeBase::GetPMove(const char * p, const unsigned int iMove)
{
	return p + iMove;
}

const void * CTreeBase::GetNodeDataFromId(unsigned int uiId)
{
	unsigned int uiPos = uiId * sizeof(XHChineseData);
	return (const void *)GetPMove(m_pzTreeWife, 4 + uiPos); //������Ҫ��4,��Ϊid��ƫ���޹�
}

//������
CTreeChinese::CTreeChinese()
{
	m_iTreeType = SELECT_TREE_CHINESE;
}

const void * CTreeChinese::GetNodeStringFromData(const XHChineseData* pData)
{
	return GetPMove(m_pcStringData, pData->uiStringPos);
}
//��ȡԭ��(ԭ����ǰ,��չ�ں�)
const char * CTreeChinese::GetNodeOrigStringFromData(const XHChineseData* pData)
{
	if(!(pData->ucAttr & FLAG_CHINESE_UCATTR_WITHORIGWORD)) {
		return NULL;
	}
	int iMove = -1;
	if(pData->ucAttr & FLAG_CHINESE_UCATTR_WITHEXTRAINFO) { //û����չ��Ϣ
		iMove = -2;
	}
	const unsigned int uiExtraPos = ((unsigned int *)GetPMove(m_pcStringData, pData->uiStringPos))[iMove];
	return GetPMove(m_pzExtraInfo, uiExtraPos);
}
const char * CTreeChinese::GetNodeExtraStringFromData(const XHChineseData* pData)
{
	if(!(pData->ucAttr & FLAG_CHINESE_UCATTR_WITHEXTRAINFO)) { //û����չ��Ϣ
		return NULL;
	}
	const unsigned int uiExtraPos = ((unsigned int *)GetPMove(m_pcStringData, pData->uiStringPos))[-1];
	return GetPMove(m_pzExtraInfo, uiExtraPos);
}
const void * CTreeChinese::GetNodeExtraInfo(const XHRbNode *pNode)
{
	const XHChineseData* sd = (XHChineseData *)GetTreeWife(pNode);
	return (const void *)GetNodeExtraStringFromData(sd);
}
const unsigned int CTreeChinese::GetIdFromNode(const XHRbNode *pNode)
{
	unsigned int ui = pNode->UncomposeWife();
	return (ui - 4) / sizeof(XHChineseData);
}

//������
CTreeSpell::CTreeSpell()
{
	m_iTreeType = SELECT_TREE_SPELL;
}

//ֻ��������������
const void * CTreeSpell::GetNodeExtraInfo(const XHRbNode *pNode)
{
	const XHSpellData * sd = (const XHSpellData *)GetTreeWife(pNode);
	return (void *)GetPMove(m_pzExtraInfo, sd->uiUpperPos);
}

const unsigned int CTreeSpell::GetNodeExtraNums(const XHRbNode *pNode)
{
	const XHSpellData * sd = (XHSpellData *)GetTreeWife(pNode);
	if(!sd->bMD) return 1; //��һ����չ��Ϣ
	//�ж����չ��Ϣ
	return ((unsigned int *)(m_pzExtraInfo + sd->uiUpperPos))[-1];
}

const void * CTreeCenterChinese::GetNodeExtraInfo(const XHSpellData *pData)
{
	return (void *)GetPMove(m_pzExtraInfo, pData->uiUpperPos);
}
const unsigned int CTreeCenterChinese::GetNodeExtraNums(const XHSpellData *pData)
{
	if(!pData->bMD) return 1; //û����չ��Ϣ
	//����չ��Ϣ
	return ((unsigned int *)(m_pzExtraInfo + pData->uiUpperPos))[-1];
}

CTreeSimple::CTreeSimple()
{
	m_iTreeType = SELECT_TREE_SIMPLE;
}

CTreeCenterChinese::CTreeCenterChinese()
{
	m_iTreeType = SELECT_TREE_CENTER_CHINESE;
}

CTreeCenterSpell::CTreeCenterSpell()
{
	m_iTreeType = SELECT_TREE_CENTER_SPELL;
}

CTreeCenterSimple::CTreeCenterSimple()
{
	m_iTreeType = SELECT_TREE_CENTER_SIMPLE;
}

CTreeExChinese::CTreeExChinese()
{
	m_iTreeType = SELECT_TREE_EX_CHINESE;
}

CTreeExSpell::CTreeExSpell()
{
	m_iTreeType = SELECT_TREE_EX_SPELL;
}

CTreeExSimple::CTreeExSimple()
{
	m_iTreeType = SELECT_TREE_EX_SIMPLE;
}

CTreeExCenterChinese::CTreeExCenterChinese()
{
	m_iTreeType = SELECT_TREE_EX_CENTER_CHINESE;
}

CTreeExCenterSpell::CTreeExCenterSpell()
{
	m_iTreeType = SELECT_TREE_EX_CENTER_SPELL;
}

CTreeExCenterSimple::CTreeExCenterSimple()
{
	m_iTreeType = SELECT_TREE_EX_CENTER_SIMPLE;
}
