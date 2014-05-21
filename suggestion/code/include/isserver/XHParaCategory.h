#ifndef _XH_PARA_CATEGORY_H_
#define _XH_PARA_CATEGORY_H_
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <string>
//���ּ򵥵������þ޴�,���ฺ���������,�����ƶ�,���ؽ������,����״̬����̵ļ�¼
#define SEARCH_KEY_TYPE_UNKNOWN            0 //δ֪
#define SEARCH_KEY_TYPE_CHINESE            1 //������
#define SEARCH_KEY_TYPE_CHINESESPELL       2 //����ƴ�����
#define SEARCH_KEY_TYPE_CHINESENUMBER      3 //�������ֻ��
#define SEARCH_KEY_TYPE_SPELL              4 //��ƴ��
#define SEARCH_KEY_TYPE_CHINESESPELLNUM    5 //����ƴ�����ֻ��
#define SEARCH_KEY_TYPE_SPELLNUM           6 //ƴ�����ֻ��
#define SEARCH_KEY_TYPE_NINEGRID           7 //�Ź���
#define SEARCH_KEY_TYPE_PURENUMBER         8 //������
#define SEARCH_KEY_TYPE_WITHDOT            9 //����dot
//#define SEARCH_KEY_TYPE_PUREFUZZY         10 //�����

#define MATCH_CATEGORY_COMMON              0 //��ͨ����
#define MATCH_CATEGORY_MIXEDSS             1 //��ƴȫƴ���->ʹ����simpleself
#define MATCH_CATEGORY_MIXEDCS             2 //����ȫƴ���
#define MATCH_CATEGORY_MIXED3              3 //��ƴȫƴ���ֻ��
#define MATCH_CATEGORY_MISTAKE             4 //����
#define MATCH_CATEGORY_SPELLCORRECT        5 //ƴ������
#define MATCH_CATEGORY_NINEGRIDKEY         6 //�Ź���
#define MATCH_CATEGORY_WITHDOT             7 //����dot

#define PATTERN_TASK_SELECT_CUR            0 //ֻʹ�õ�ǰtask
#define PATTERN_TASK_SELECT_ALL            1 //ʹ�����е�task

//�趨һЩ��ѯ���ԵĿ���
//��λ���д����
#define SFFC_WITH_MixSearch           1 //�Ƿ�֧�ֻ������
#define SFFC_WITH_ChineseCorrect      2 //�Ƿ�֧�ֺ���תƴ���ľ���
#define SFFC_WITH_ManyTaskSearch      3 //�Ƿ��Ƕ�������ѯ֧�� 
#define SFFC_WITH_ChineseSpellCorrect 4 //�Ƿ�֧�ֺ���ȫƴ��ϲ�ѯ
#define SFFC_WITH_SpellSimpleCorrect  5 //�Ƿ�֧��ȫƴ��ƴ��ϲ�ѯ

using namespace std;

#define U32 unsigned int 
typedef struct _ParaOrig {
	//����λ���м���ı���ŵ���ǰ��
	U32    bNeedExtraInfo:1;            //�Ƿ���Ҫ������չ��Ϣ  ////���λ  unsigned int = 1
	U32    bSupportDot:1;               //�Ƿ���Ҫ֧��dot��ѯ
	U32    bSupportMix:1;               //�Ƿ���Ҫ֧�ֻ��(����,ƴ��)��ѯ
	U32    bSupportSpellCorrect:1;      //֧��ƴд����
	U32    bSupportSpellCase:1;         //֧�ִ�Сд����
	U32    bPreRegular:1;               //ǰ׺֧�ּ�����
	//U32    bNineGrid:1;                 //�Ź���
	U32    unuse:25;
	int    iResNumLimit;              //Ҫ�󷵻صĽ������
	int    iFlagLow;                  //flag������(����)
	int    iFlagUp;                   //flag������(����)
	int    iFlagOr;                   //or��Ϊ0,����
	int    iFlagAnd;                  //and��Ϊ0,����
	int    iPreNullSize;              //ǰ׺�յĳ���(ģ��ƥ��)
	bool   bSameLen;                  //���صĽ������Ҫ����ǰ����ĳ���һ��
	bool   bNineGrid;                 //ʹ�þŹ�֧��(���������Զ���)
	int    iSysFlagForCategory;       //�趨һЩ��ѯ���ԵĿ���
	int    iTreeSelect;               //ѡ����Щ������ʹ��
                                      
	string strPre;                    //��ѯ��ǰ׺
	string strKey;                    //ԭʼ�Ĳ�ѯ��
}ParaOrig;

//һЩϵͳ�ı�־,Ϊ��ָ����ѯ����ʹ��,��Щ��־��Ӱ������ƶ�
typedef struct _SysFlagForCategory {
	bool bWithMixSearch; //�Ƿ�֧�ֻ������
	bool bWithChineseCorrect; //�Ƿ�֧�ֺ���תƴ���ľ���
	bool bWithManyTaskSearch;  //�Ƿ��Ƕ�������ѯ֧�� 
	bool bWithChineseSpellCorrect; //�Ƿ�֧�ֺ���ȫƴ��ϲ�ѯ
	bool bWithSpellSimpleCorrect;  //�Ƿ�֧��ȫƴ��ƴ��ϲ�ѯ
}SysFlagForCategory;

//0����flag
typedef struct _FlagSet {
	bool bWithFlag;
	int iFlagLow;  //���ĸ�Ϊһ�����壬���Ե���ʹ�ã�Ҳ���Ի��ʹ�ã�0����û������
	int iFlagUp;
	int iFlagAnd;
	int iFlagOr;
	int iFlagEqual; //��Ӧflg���������,������ ����������ĸ�ʵЧ
	bool IsGoodFlag(int iFlag) {
		if(!bWithFlag)  return true;
		if(iFlagEqual) {
			if(iFlag == iFlagEqual) return true;
			return false;
		}
		if(iFlagLow && iFlag < iFlagLow) return false;
		if(iFlagUp  && iFlag > iFlagUp)  return false;
		if(iFlagAnd && !(iFlagAnd & iFlag)) return false;
		return true;
	};
}FlagSet;

//һ������ִ�еĲ���
typedef struct _XHExePara {
	char cTreeType;      //��������
	char cMatchCategory; //ƥ�����
	unsigned short  usStopNum; //�ò��������Ľ�����ܵ��������ֹͣ��������Ĳ���
	int  iTaskSelect;    //PATTERN_TASK_SELECT_CUR; PATTERN_TASK_SELECT_ALL
	int  iAddWeight;     //�������ò���,�ϼ������в���Ӧ�����Ӷ���Ȩ��,��֤֮ǰ�Ѿ���ѯ�����Ľ��������ǰ��
	string strKey;       //Ҫ����ƥ����ַ���
	ParaOrig * pParaOrig;
	
	void I(char cTree, char cMatch, int iPatternSelect, unsigned short usLimit, int iWeight, string & strK, ParaOrig *p) {
		cTreeType = cTree;
		cMatchCategory = cMatch;
		usStopNum = usLimit;
		iAddWeight = iWeight;
		iTaskSelect = iPatternSelect;
		strKey = strK;
		pParaOrig = p;
	};
	void SetMixKey(const string & strK) {
		strKey = strK;
	};
	int GetTotalLen() {
		//remark--------->>>>need remember
		return strKey.size() + pParaOrig->strPre.size() + pParaOrig->iPreNullSize;
		return pParaOrig->strPre.size() + pParaOrig->strKey.size() + pParaOrig->iPreNullSize;
	};
	void Print() {
		printf("\n-----------------------------\n");
		printf("tree type->%d\n", cTreeType);
		printf("cMatchCategory->%d\n", cMatchCategory);
		printf("limit stop->%d\n", usStopNum);
		printf("key string->%s\n", strKey.c_str());
		printf("---orig---\n");
		printf("   pre:%s\n", pParaOrig->strPre.c_str());
		printf("   key:%s\n", pParaOrig->strKey.c_str());
		printf("   pre null size:%d\n", pParaOrig->iPreNullSize);
	};
}XHExePara;

class CXHPara {
public:
	CXHPara(map<const string, string> & mapInit, SysFlagForCategory * pSysFlag) {
		m_mapInitSave = mapInit;
		m_pSysFlag = pSysFlag;
		LoadOnce();
		Init();
	};
	string GetTaskName() {
		return m_strTaskName;
	};
	string GetPre() {
		return m_stParaOrig.strPre;
	};
	int GetLimitNumber() {
		return m_stParaOrig.iResNumLimit;
	};
	//���ǰ׺Ϊ0������Ϊֱ������ȫ��task�Ĳ���
	int GetLikeAllTask() {
		return (m_stParaOrig.strPre == "0") ? 1 : 0;
	};
	int GetCategoryNum() {
		return m_vecCategoryList.size();
	};
	XHExePara * GetCategory(int iIdx) {
		return &(m_vecCategoryList[iIdx]);
	};
	bool GetMixKey(const string & strIn, string & strMixKey);
	FlagSet    stFlagSet;
	int  iMustAllTask;   //ǿ��ʹ��ȫ���������ѯ�����ȼ��ǳ��ߣ�ǿ��
	bool bSimpleView;   //������ʾ�Ƿ��Ǽ���ʾ
private:
	bool     m_bWithMixSerach; //�������ļ��ж�ȡ���Ƿ������н����˻�ϲ�ѯ���ֵ�����
	char     m_cSearchKeyType;
	string   m_strKeyFormat;
	string   m_strTaskName;
	ParaOrig m_stParaOrig;
	map<const string, string> m_mapInitSave;
	int m_iCategoryNum; //�ܹ��Ĳ�����Ŀ

	vector<XHExePara> m_vecCategoryList;
	SysFlagForCategory * m_pSysFlag;

private:
	string StringDotFormat(const string & strKey, string & strSimple);
	void Init_SetupCategory();
	void Init_SearchKeyAnly();
	void Init_ParaAnly();
	void Init();
	void LoadOnce(); //����һ��
	unsigned int ToInt(const string & strKey, unsigned int uiMin, unsigned int uiMax, unsigned int uiDefault);
	string ToString(const string & strKey);
};



#endif
