#include "isserver/XHParaCategory.h"
#include <stdlib.h>
#include "common/SysCommon.h"
#include "util/XHStrUtils.h"
#include "util/Pinyin.h"

//标志的最大值
#define NUMBER_MAX_FLAG_LIMIT  0xffff

unsigned int CXHPara::ToInt(const string & strKey, unsigned int uiMin, unsigned int uiMax, unsigned int uiDefault)
{
	string strResult = ToString(strKey);
	if(strResult == "") return uiDefault;
	unsigned int uiRet = atoi(strResult.c_str());
	if(uiRet < uiMin) return uiMin;
	if(uiRet > uiMax) return uiMax;
	return uiRet;
}
string CXHPara::ToString(const string & strKey)
{
	string strResult = "";
	map<const string, string>::iterator iIter;
	iIter = m_mapInitSave.find(strKey);
	if(iIter != m_mapInitSave.end()) {
		strResult = iIter->second;
	}
	return strResult;
}

void CXHPara::Init()
{
	Init_ParaAnly();
	Init_SearchKeyAnly();
	Init_SetupCategory();
}


void CXHPara::LoadOnce()
{
	static int iOnce = 0;
	if(iOnce) return ;
	iOnce = 1;
}

//bSameLen设置了之后,不再查找中间的一些树{center}
#define XH_SETUP_CATEGORY(tree,match,p, limit,weight, key) { \
	if(m_stParaOrig.iTreeSelect & (1 << tree)) { \
		if(!(m_stParaOrig.bSameLen && (tree == SELECT_TREE_CENTER_CHINESE || tree == SELECT_TREE_CENTER_SPELL || tree == SELECT_TREE_CENTER_SIMPLE \
		 || tree == SELECT_TREE_EX_CENTER_CHINESE || tree == SELECT_TREE_EX_CENTER_SPELL || tree == SELECT_TREE_EX_CENTER_SIMPLE))) { \
			xepTmp.I(tree, match,p, limit, weight, key, &m_stParaOrig); \
			m_vecCategoryList.push_back(xepTmp); \
		} \
	} \
}
#define WITH_CATEGORY(p) m_pSysFlag->bWith##p && (iSysFlagForCategory & (1<<SFFC_WITH_##p))
	
void CXHPara::Init_SetupCategory()
{
	XHExePara xepTmp;
	string strOrigKey = m_stParaOrig.strKey;
	int    iLimit = m_stParaOrig.iResNumLimit;
	string strSpell = "";
	string strSpell1 = "";
	string strSpell2 = "";
	string strDotSimple = "";
	string strDot;
	int iSysFlagForCategory = m_stParaOrig.iSysFlagForCategory;

	int iWeight = 10000;
	switch(m_cSearchKeyType) {
		case SEARCH_KEY_TYPE_CHINESE:
		case SEARCH_KEY_TYPE_CHINESENUMBER: 
		case SEARCH_KEY_TYPE_CHINESESPELLNUM: //汉字拼音数字这么复杂的混合暂时先不走混合查询
			//汉字树 当前任务 普通查询
			XH_SETUP_CATEGORY(SELECT_TREE_CHINESE,           MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strOrigKey);
			XH_SETUP_CATEGORY(SELECT_TREE_CENTER_CHINESE,    MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strOrigKey);
			if(WITH_CATEGORY(ManyTaskSearch)) { //支持全部task一起查询
				XH_SETUP_CATEGORY(SELECT_TREE_EX_CHINESE,           MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strOrigKey);
				XH_SETUP_CATEGORY(SELECT_TREE_EX_CENTER_CHINESE,    MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strOrigKey);
			}
			if(WITH_CATEGORY(ChineseCorrect)) { //支持纠错查询
				strSpell = Pinyin::getInstance().getPinyin(strOrigKey, true);
				strSpell[0] = (strSpell[0] >= 'a' && strSpell[0] <= 'z') ? (strSpell[0] - 'a' + 'A') : strSpell[0];
				XH_SETUP_CATEGORY(SELECT_TREE_SPELL,           MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strSpell);
				XH_SETUP_CATEGORY(SELECT_TREE_CENTER_SPELL,    MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strSpell);
				if(WITH_CATEGORY(ManyTaskSearch)) { //支持全部task一起查询
					XH_SETUP_CATEGORY(SELECT_TREE_EX_SPELL,           MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strSpell);
					XH_SETUP_CATEGORY(SELECT_TREE_EX_CENTER_SPELL,    MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strSpell);
				}
			}
			break;
		case SEARCH_KEY_TYPE_CHINESESPELL:
			XH_SETUP_CATEGORY(SELECT_TREE_CHINESE,           MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strOrigKey);
			XH_SETUP_CATEGORY(SELECT_TREE_CENTER_CHINESE,    MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strOrigKey);
			if(WITH_CATEGORY(ManyTaskSearch)) { //支持全部task一起查询
				XH_SETUP_CATEGORY(SELECT_TREE_EX_CHINESE,        MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strOrigKey);
				XH_SETUP_CATEGORY(SELECT_TREE_EX_CENTER_CHINESE, MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strOrigKey);
			}
			if(WITH_CATEGORY(ChineseSpellCorrect)) { //这个特别复杂,容易出错,脑子清醒的时候处理
				strSpell = Pinyin::getInstance().getPinyin(strOrigKey, true);
				string strTail = "";
				for(int iLoop = 0; iLoop < strOrigKey.size(); iLoop++) {
					char c = strOrigKey[iLoop];
					if(c & 0x80) {
						strTail += c;
					}
				}
				if((strTail.size() % 2) == 0 && strTail.size() > 0) {
					strTail = strSpell + "|" + strTail;
					XH_SETUP_CATEGORY(SELECT_TREE_SPELL,        MATCH_CATEGORY_MIXEDCS, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strTail);
					XH_SETUP_CATEGORY(SELECT_TREE_CENTER_SPELL, MATCH_CATEGORY_MIXEDCS, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strTail);
					if(WITH_CATEGORY(ManyTaskSearch)) { //支持全部task一起查询
						XH_SETUP_CATEGORY(SELECT_TREE_EX_SPELL,        MATCH_CATEGORY_MIXEDCS, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strTail);
						XH_SETUP_CATEGORY(SELECT_TREE_EX_CENTER_SPELL, MATCH_CATEGORY_MIXEDCS, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strTail);
					}
				}
			}
			if(WITH_CATEGORY(ChineseCorrect)) {
				if(strSpell == "") {
					strSpell = Pinyin::getInstance().getPinyin(strOrigKey, true);
				}
				strSpell[0] = (strSpell[0] >= 'a' && strSpell[0] <= 'z') ? (strSpell[0] - 'a' + 'A') : strSpell[0];
				XH_SETUP_CATEGORY(SELECT_TREE_SPELL,           MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strSpell);
				XH_SETUP_CATEGORY(SELECT_TREE_CENTER_SPELL,    MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strSpell);
				if(WITH_CATEGORY(ManyTaskSearch)) { //支持全部task一起查询
					XH_SETUP_CATEGORY(SELECT_TREE_EX_SPELL,        MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strSpell);
					XH_SETUP_CATEGORY(SELECT_TREE_EX_CENTER_SPELL, MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strSpell);
				}
			}
			break;
		case SEARCH_KEY_TYPE_SPELL:
			XH_SETUP_CATEGORY(SELECT_TREE_SPELL,         MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strOrigKey);
			XH_SETUP_CATEGORY(SELECT_TREE_SIMPLE,        MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, 0,       strOrigKey);
			XH_SETUP_CATEGORY(SELECT_TREE_CENTER_SPELL,  MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strOrigKey);
			XH_SETUP_CATEGORY(SELECT_TREE_CENTER_SIMPLE, MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, 0,       strOrigKey);
			if(WITH_CATEGORY(ManyTaskSearch)) { //支持全部task一起查询
				XH_SETUP_CATEGORY(SELECT_TREE_EX_SPELL,         MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strOrigKey);
				XH_SETUP_CATEGORY(SELECT_TREE_EX_SIMPLE,        MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, 0,       strOrigKey);
				XH_SETUP_CATEGORY(SELECT_TREE_EX_CENTER_SPELL,  MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strOrigKey);
				XH_SETUP_CATEGORY(SELECT_TREE_EX_CENTER_SIMPLE, MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, 0,       strOrigKey);
			}
			if(WITH_CATEGORY(SpellSimpleCorrect)) { //支持简拼全拼混合
				if(!GetMixKey(strOrigKey, strSpell2)) { //不能获取简拼
					break;
				}
				XH_SETUP_CATEGORY(SELECT_TREE_SIMPLE,        MATCH_CATEGORY_MIXEDSS, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strSpell2);
				XH_SETUP_CATEGORY(SELECT_TREE_CENTER_SIMPLE, MATCH_CATEGORY_MIXEDSS, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strSpell2);
				if(WITH_CATEGORY(ManyTaskSearch)) { //支持全部task一起查询
					XH_SETUP_CATEGORY(SELECT_TREE_EX_SIMPLE,        MATCH_CATEGORY_MIXEDSS, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strSpell2);
					XH_SETUP_CATEGORY(SELECT_TREE_EX_CENTER_SIMPLE, MATCH_CATEGORY_MIXEDSS, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strSpell2);
				}
			}
			break;
		case SEARCH_KEY_TYPE_SPELLNUM:
			XH_SETUP_CATEGORY(SELECT_TREE_SPELL,         MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strOrigKey);
			XH_SETUP_CATEGORY(SELECT_TREE_SIMPLE,        MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, 0,       strOrigKey);
			XH_SETUP_CATEGORY(SELECT_TREE_CENTER_SPELL,  MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strOrigKey);
			XH_SETUP_CATEGORY(SELECT_TREE_CENTER_SIMPLE, MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, 0,       strOrigKey);
			if(WITH_CATEGORY(ManyTaskSearch)) { //支持全部task一起查询
				XH_SETUP_CATEGORY(SELECT_TREE_EX_SPELL,         MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strOrigKey);
				XH_SETUP_CATEGORY(SELECT_TREE_EX_SIMPLE,        MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, 0,       strOrigKey);
				XH_SETUP_CATEGORY(SELECT_TREE_EX_CENTER_SPELL,  MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strOrigKey);
				XH_SETUP_CATEGORY(SELECT_TREE_EX_CENTER_SIMPLE, MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, 0,       strOrigKey);
			}
			break;
		case SEARCH_KEY_TYPE_NINEGRID:
			XH_SETUP_CATEGORY(SELECT_TREE_SPELL,         MATCH_CATEGORY_NINEGRIDKEY, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strOrigKey);
			XH_SETUP_CATEGORY(SELECT_TREE_SIMPLE,        MATCH_CATEGORY_NINEGRIDKEY, PATTERN_TASK_SELECT_CUR, iLimit, 0,       strOrigKey);
			if(WITH_CATEGORY(ManyTaskSearch)) { //支持全部task一起查询
				XH_SETUP_CATEGORY(SELECT_TREE_EX_SPELL,         MATCH_CATEGORY_NINEGRIDKEY, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strOrigKey);
				XH_SETUP_CATEGORY(SELECT_TREE_EX_SIMPLE,        MATCH_CATEGORY_NINEGRIDKEY, PATTERN_TASK_SELECT_ALL, iLimit, 0,       strOrigKey);
			}
			break;
		case SEARCH_KEY_TYPE_PURENUMBER:
			XH_SETUP_CATEGORY(SELECT_TREE_CHINESE,           MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strOrigKey);
			XH_SETUP_CATEGORY(SELECT_TREE_CENTER_CHINESE,    MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strOrigKey);
			if(WITH_CATEGORY(ManyTaskSearch)) { //支持全部task一起查询
				XH_SETUP_CATEGORY(SELECT_TREE_EX_CHINESE,           MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strOrigKey);
				XH_SETUP_CATEGORY(SELECT_TREE_EX_CENTER_CHINESE,    MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strOrigKey);
			}
			break;
		case SEARCH_KEY_TYPE_WITHDOT:
			strDot = StringDotFormat(strOrigKey, strDotSimple);
			XH_SETUP_CATEGORY(SELECT_TREE_SPELL,         MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strDot);
			XH_SETUP_CATEGORY(SELECT_TREE_SIMPLE,        MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, 0,       strDotSimple);
			XH_SETUP_CATEGORY(SELECT_TREE_CENTER_SPELL,  MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, iWeight, strDot);
			XH_SETUP_CATEGORY(SELECT_TREE_CENTER_SIMPLE, MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_CUR, iLimit, 0,       strDotSimple);
			if(WITH_CATEGORY(ManyTaskSearch)) { //支持全部task一起查询
				XH_SETUP_CATEGORY(SELECT_TREE_EX_SPELL,         MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strDot);
				XH_SETUP_CATEGORY(SELECT_TREE_EX_SIMPLE,        MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, 0,       strDotSimple);
				XH_SETUP_CATEGORY(SELECT_TREE_EX_CENTER_SPELL,  MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, iWeight, strDot);
				XH_SETUP_CATEGORY(SELECT_TREE_EX_CENTER_SIMPLE, MATCH_CATEGORY_COMMON, PATTERN_TASK_SELECT_ALL, iLimit, 0,       strDotSimple);
			}
			break;
		default: //case SEARCH_KEY_TYPE_UNKNOWN:
			//XH_SETUP_CATEGORY(SELECT_TREE_CHINESE,            MATCH_CATEGORY_COMMON,        iLimit, strOrigKey); //汉字树   & 普通检索
			break;
	}
}
#undef XH_SETUP_CATEGORY

string CXHPara::StringDotFormat(const string & strKey, string & strSimple)
{
	string strRet = "";
	int iLoop;
	int iLen = strKey.size();
	int iBegin, iEnd;
	for(iLoop = 0; iLoop < iLen; iLoop++) {
		if(strKey[iLoop] != '\'') {
			break;
		}
	}
	iBegin = iLoop;
	for(iLoop = iLen - 1; iLoop > iBegin; iLoop--) {
		if(strKey[iLoop] != '\'') {
			break;
		}
	}
	iEnd = iLoop;
	bool bDot = true;
	for(iLoop = iBegin; iLoop <= iEnd; iLoop++) {
		if(strKey[iLoop] == '\'') {
			bDot = true;
		} else if(bDot) {
			strRet += strKey[iLoop] - 'a' + 'A';
			strSimple += strKey[iLoop];
			bDot = false;
		} else {
			strRet += strKey[iLoop];
			strSimple += strKey[iLoop];
		}
	}
	return strRet;
}
void CXHPara::Init_ParaAnly()
{
	unsigned uiFlags = ToInt("attr", 0, 0xffffffff, 0);
	((unsigned int*)&m_stParaOrig)[0] = uiFlags;
	m_stParaOrig.iResNumLimit  = ToInt("lmt", 1, NUMBER_MAX_RETURN_LIMIT, NUMBER_RETURN_DEFAULT);
	m_stParaOrig.iFlagLow      = ToInt("fl",  0, NUMBER_MAX_RETURN_LIMIT, 0);
	m_stParaOrig.iFlagUp       = ToInt("fu",  0, NUMBER_MAX_RETURN_LIMIT, 0);
	m_stParaOrig.iFlagOr       = ToInt("fo",  0, NUMBER_MAX_RETURN_LIMIT, 0);
	m_stParaOrig.iFlagAnd      = ToInt("fa",  0, NUMBER_MAX_RETURN_LIMIT, 0);
	m_stParaOrig.iPreNullSize  = ToInt("pns", 0, NUMBER_MAX_RETURN_LIMIT, 0);
	m_stParaOrig.bNineGrid     = ToInt("wng", 0, 1, 0); //暂时打开九宫支持 
	iMustAllTask  = ToInt("mat", 0, 1, 0); //强制使用全部的ｔａｓｋ检索
	m_stParaOrig.bSameLen      = ToInt("sl",  0, 1, 0);
	m_stParaOrig.iTreeSelect   = ToInt("ts",  0, 0xffffffff, 0xffffffff);
	m_stParaOrig.iSysFlagForCategory = ToInt("sffc", 0, 0xffffffff, 0xffffffff);

	//标志
	int iFlag = ToInt("flg", 0, NUMBER_MAX_FLAG_LIMIT, 0);
	if(iFlag != 0) {
		stFlagSet.bWithFlag  = true;
		stFlagSet.iFlagEqual = iFlag;
	} else {
		stFlagSet.iFlagLow = ToInt("flgl", 0, NUMBER_MAX_FLAG_LIMIT, 0);
		stFlagSet.iFlagUp  = ToInt("flgu", 0, NUMBER_MAX_FLAG_LIMIT, 0);
		stFlagSet.iFlagAnd = ToInt("flga", 0, NUMBER_MAX_FLAG_LIMIT, 0);
		stFlagSet.iFlagOr  = ToInt("flgo", 0, NUMBER_MAX_FLAG_LIMIT, 0); //暂时不用
		if(stFlagSet.iFlagLow == 0 && stFlagSet.iFlagUp == 0 && stFlagSet.iFlagAnd == 0 && stFlagSet.iFlagOr == 0) {
			stFlagSet.bWithFlag  = false;
		} else {
			stFlagSet.bWithFlag  = true;
		}
	}
	m_stParaOrig.strPre       = ToString("p");
	string strView = ToString("view");
	if(strView == "simple") {
		bSimpleView = true;
	} else {
		bSimpleView = false;
	}
	m_strTaskName       = ToString("x");
	string strKey       = ToString("w");
	//取出空格,噪音，----------------->>>>>>有待优化
	int iLoop, iSize = strKey.size();
	m_stParaOrig.strKey = "";
	for(iLoop = 0; iLoop < iSize; iLoop++) {
		char c = strKey[iLoop];
		if(c & 0x80) {
			if(iLoop != iSize - 1) {
				m_stParaOrig.strKey += c;
				iLoop++;
				c = strKey[iLoop];
				m_stParaOrig.strKey += c;
			} else {
				break;
			}
		} else if(c >= 'A' && c <= 'Z') {
			c = c - 'A' + 'a';
			m_stParaOrig.strKey += c;
		} else if(c >= 'a' && c <= 'z' || c >= '0' && c <= '9' || c == '\'') {
			m_stParaOrig.strKey += c;
		}
	}
	
}

void CXHPara::Init_SearchKeyAnly()
{
	bool bWithDot     = false;
	bool bWithNum     = false;
	bool bWithChar    = false;
	bool bWithChinese = false;
	string strTmp = m_stParaOrig.strKey;
	int iLoop, iSize = strTmp.size();
	string strNew = "";
	for(iLoop = 0; iLoop < iSize; iLoop++) {
		char c = strTmp[iLoop];
		if(c & 0x80) { //可以直接往后走两步,但是可能传入的字符串本身就不完整,可能一半汉字
			bWithChinese = true;
			strNew += strTmp[iLoop];
		} else if(c >= '0' && c <= '9') {
			bWithNum = true;
			strNew += strTmp[iLoop];
		} else if(c >= 'A' && c <= 'Z') {
			bWithChar = true;
			strNew += (c - 'A' + 'a');
		} else if(c >= 'a' && c <= 'z') {
			bWithChar = true;
			strNew += c;
		} else if(c == '\'') {
			bWithDot = true;
		}
	}
	char cRetType = SEARCH_KEY_TYPE_UNKNOWN;
	if(bWithChinese) {
		if(bWithChar && bWithNum) {
			cRetType = SEARCH_KEY_TYPE_CHINESESPELLNUM;
		} else if(bWithChar) {
			cRetType = SEARCH_KEY_TYPE_CHINESESPELL;
		} else if(bWithNum) {
			cRetType = SEARCH_KEY_TYPE_CHINESENUMBER;
		} else {
			cRetType = SEARCH_KEY_TYPE_CHINESE;
		}
	} else if(bWithChar) {
		if(bWithNum) {
			cRetType = SEARCH_KEY_TYPE_SPELLNUM;
		} else if(bWithDot) { //现在的dot只支持跟纯拼音一起的
			cRetType = SEARCH_KEY_TYPE_WITHDOT;
		} else {
			cRetType = SEARCH_KEY_TYPE_SPELL;
		}
	} else if(bWithNum) {
		if(m_stParaOrig.bNineGrid) {
			cRetType = SEARCH_KEY_TYPE_NINEGRID;
		} else {
			cRetType = SEARCH_KEY_TYPE_PURENUMBER;
		}
	} else {
		cRetType = SEARCH_KEY_TYPE_UNKNOWN;
	}
	if(cRetType != SEARCH_KEY_TYPE_WITHDOT) {
		m_strKeyFormat = strNew;
	} else {
		m_strKeyFormat = m_stParaOrig.strKey;
	}
	m_cSearchKeyType = cRetType;
}
bool CXHPara::GetMixKey(const string & strIn, string & strMixKey)
{
	string strOrigKey = strIn;
	vector<string> vecSplitPinyin;
	Pinyin::getInstance().splitPinyinWithSimple(vecSplitPinyin, strOrigKey.c_str(), strOrigKey.size());
	string strSpell = "";
	if(vecSplitPinyin.size() > 0 && strOrigKey.size() > 2) {
		int iSingle = 0, iMul = 0;
		strSpell = "";
		vector<string> vecSplit;
		XHStrUtils::StrTokenize(vecSplit, vecSplitPinyin[0], " ");
		int iLoop, iSize = vecSplit.size();
		for(iLoop = 0; iLoop < iSize; iLoop++) {
			if(vecSplit[iLoop].size() == 1) {
				if(iLoop != iSize - 1) {
					iSingle++;
				}
			} else {
				iMul++;
			}
			string strNew = vecSplit[iLoop];
			if(strNew[0] >= 'a' && strNew[0] <= 'z') strNew[0] = (strNew[0] - 'a' + 'A');
			strSpell += strNew;
		}
		if(iSingle && iMul) {
			strMixKey = strSpell;
			return true;
		}
	}
	return false;
}
