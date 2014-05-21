#ifndef _XH_PARA_CATEGORY_H_
#define _XH_PARA_CATEGORY_H_
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include <string>
//名字简单但是作用巨大,该类负责参数解析,策略制定,返回结果保存,运行状态与过程的记录
#define SEARCH_KEY_TYPE_UNKNOWN            0 //未知
#define SEARCH_KEY_TYPE_CHINESE            1 //纯汉字
#define SEARCH_KEY_TYPE_CHINESESPELL       2 //汉字拼音混合
#define SEARCH_KEY_TYPE_CHINESENUMBER      3 //汉字数字混合
#define SEARCH_KEY_TYPE_SPELL              4 //纯拼音
#define SEARCH_KEY_TYPE_CHINESESPELLNUM    5 //汉字拼音数字混合
#define SEARCH_KEY_TYPE_SPELLNUM           6 //拼音数字混合
#define SEARCH_KEY_TYPE_NINEGRID           7 //九宫格
#define SEARCH_KEY_TYPE_PURENUMBER         8 //纯数字
#define SEARCH_KEY_TYPE_WITHDOT            9 //含有dot
//#define SEARCH_KEY_TYPE_PUREFUZZY         10 //纯混合

#define MATCH_CATEGORY_COMMON              0 //普通检索
#define MATCH_CATEGORY_MIXEDSS             1 //简拼全拼混合->使用了simpleself
#define MATCH_CATEGORY_MIXEDCS             2 //汉字全拼混合
#define MATCH_CATEGORY_MIXED3              3 //简拼全拼汉字混合
#define MATCH_CATEGORY_MISTAKE             4 //纠错
#define MATCH_CATEGORY_SPELLCORRECT        5 //拼音纠错
#define MATCH_CATEGORY_NINEGRIDKEY         6 //九宫格
#define MATCH_CATEGORY_WITHDOT             7 //含有dot

#define PATTERN_TASK_SELECT_CUR            0 //只使用当前task
#define PATTERN_TASK_SELECT_ALL            1 //使用所有的task

//设定一些查询策略的开关
//按位进行处理的
#define SFFC_WITH_MixSearch           1 //是否支持混合搜索
#define SFFC_WITH_ChineseCorrect      2 //是否支持汉字转拼音的纠错
#define SFFC_WITH_ManyTaskSearch      3 //是否是多个任务查询支持 
#define SFFC_WITH_ChineseSpellCorrect 4 //是否支持汉字全拼混合查询
#define SFFC_WITH_SpellSimpleCorrect  5 //是否支持全拼简拼混合查询

using namespace std;

#define U32 unsigned int 
typedef struct _ParaOrig {
	//按照位进行计算的必须放到最前面
	U32    bNeedExtraInfo:1;            //是否需要返回扩展信息  ////最低位  unsigned int = 1
	U32    bSupportDot:1;               //是否需要支持dot查询
	U32    bSupportMix:1;               //是否需要支持混合(汉字,拼音)查询
	U32    bSupportSpellCorrect:1;      //支持拼写纠错
	U32    bSupportSpellCase:1;         //支持大小写敏感
	U32    bPreRegular:1;               //前缀支持简单正则
	//U32    bNineGrid:1;                 //九宫格
	U32    unuse:25;
	int    iResNumLimit;              //要求返回的结果个数
	int    iFlagLow;                  //flag的下限(包含)
	int    iFlagUp;                   //flag的上限(包含)
	int    iFlagOr;                   //or不为0,符合
	int    iFlagAnd;                  //and不为0,符合
	int    iPreNullSize;              //前缀空的长度(模糊匹配)
	bool   bSameLen;                  //返回的结果长度要跟当前输入的长度一致
	bool   bNineGrid;                 //使用九宫支持(条件允许自动打开)
	int    iSysFlagForCategory;       //设定一些查询策略的开关
	int    iTreeSelect;               //选定哪些树可以使用
                                      
	string strPre;                    //查询的前缀
	string strKey;                    //原始的查询词
}ParaOrig;

//一些系统的标志,为了指定查询策略使用,这些标志将影响策略制定
typedef struct _SysFlagForCategory {
	bool bWithMixSearch; //是否支持混合搜索
	bool bWithChineseCorrect; //是否支持汉字转拼音的纠错
	bool bWithManyTaskSearch;  //是否是多个任务查询支持 
	bool bWithChineseSpellCorrect; //是否支持汉字全拼混合查询
	bool bWithSpellSimpleCorrect;  //是否支持全拼简拼混合查询
}SysFlagForCategory;

//0不是flag
typedef struct _FlagSet {
	bool bWithFlag;
	int iFlagLow;  //这四个为一个整体，可以单独使用，也可以混合使用，0代表没有设置
	int iFlagUp;
	int iFlagAnd;
	int iFlagOr;
	int iFlagEqual; //对应flg，级别最高,设置了 这个其他的四个实效
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

//一个策略执行的参数
typedef struct _XHExePara {
	char cTreeType;      //树的类型
	char cMatchCategory; //匹配策略
	unsigned short  usStopNum; //该策略期望的结果数能到达多少则停止继续下面的策略
	int  iTaskSelect;    //PATTERN_TASK_SELECT_CUR; PATTERN_TASK_SELECT_ALL
	int  iAddWeight;     //如果加入该策略,上级的所有策略应该增加多少权重,保证之前已经查询出来的结果能排在前面
	string strKey;       //要进行匹配的字符串
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
	//如果前缀为0，则认为直接搜索全国task的部分
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
	int  iMustAllTask;   //强制使用全部的任务查询，优先级非常高，强制
	bool bSimpleView;   //最后的显示是否是简单显示
private:
	bool     m_bWithMixSerach; //从配置文件中读取，是否索引中建立了混合查询部分的内容
	char     m_cSearchKeyType;
	string   m_strKeyFormat;
	string   m_strTaskName;
	ParaOrig m_stParaOrig;
	map<const string, string> m_mapInitSave;
	int m_iCategoryNum; //总共的策略数目

	vector<XHExePara> m_vecCategoryList;
	SysFlagForCategory * m_pSysFlag;

private:
	string StringDotFormat(const string & strKey, string & strSimple);
	void Init_SetupCategory();
	void Init_SearchKeyAnly();
	void Init_ParaAnly();
	void Init();
	void LoadOnce(); //加载一次
	unsigned int ToInt(const string & strKey, unsigned int uiMin, unsigned int uiMax, unsigned int uiDefault);
	string ToString(const string & strKey);
};



#endif
