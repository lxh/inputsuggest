#ifndef _CREATE_DICT_H_
#define _CREATE_DICT_H_
#include <vector>
#include <string>
#include <map>
#include "util/SysConfigSet.h"
#include "util/WriteBigFile.h"

using namespace std;
#define DICT_MAX_SIZE 32

typedef struct _WordToSpell {
	string strWord;
	map<string, vector<string> > mapSimple;
	vector<string> vecFull;
}WordToSpell;

typedef struct _RecordInfo {
	int iWeight;
	vector<string> vecIds; //¶ÔÓ¦µÄºº×ÖÏÂ±ê
	vector<string> vecExtra2; //À©Õ¹ĞÅÏ¢¡
}RecordInfo;

class CCreateDict {
public:
	int Run(const string & strTaskName, CSysConfigSet *pclsSysConfigSet);

private:
	int Process();
	int CreateAllDict_First();
private:
	int WriteOneDict(const string & strPre, const string & strKey, CWriteBigFile & wFD, const int iWeight, const int iCount, const string & strExtra2 = "");
	void Init();
	void InitParamter();
	void InitLoadSplitProc();
	void SplitWord(const string & strIn, vector<string> &vecOut);
	void SplitWordNoPinyin(const string & strIn, vector<WordToSpell> & vecPinyin);
	void SplitWordWithPinyin(const string & strIn, vector<WordToSpell> & vecPinyin);
	int WriteDictFirst(const string & strPre, const string & strExPre, const string & strKey, CWriteBigFile * & writeFds, const int iWeight, const int iCount);
	int FormatJoinOneFile(const int iFieldId, map<const string, RecordInfo> & mapRecord);
	int WriteJoinOneFile(const int iFieldId, map<const string, RecordInfo> & mapRecord);
	int CreateAllDict_Second();

private:
	CSysConfigSet * m_pSysConfigSet; //ÅäÖÃÎÄ¼şµÄĞÅÏ¢
	string          m_strTaskName;   //ÈÎÎñµÄÃû³Æ
	string          m_strDataPath;   //Êı¾İµÄÂ·¾¶
	bool            m_bDictNeedCreate[DICT_MAX_SIZE]; //ĞèÒª´¦ÀíµÄ´Êµä
	bool            m_bNeedPinyinDeal; //ÊÇ·ñº¬ÓĞÆ´ÒôµÄ´¦Àí£¨Èç¹ûÃ»ÓĞÆ´Òô£¬ÄÇÃ´¾Í²»ĞèÒª»ñµÃÆ´Òô)

	//¹ØÓÚ·Ö´ÊµÄ¼¸¸ö²ÎÊı
	int  m_iMinSplitCenterWordLen;   //ÖĞ¼ä²éÑ¯µÄ´Ê×î¶Ì³¤¶ÈÎª¶àÉÙ
	int  m_iMinSplitCenterSearchLen; //×î¶ÌµÄ¿ÉÒÔ½øĞĞ·Ö´ÊµÄ³¤¶È
	int  m_iMaxSplitCenterSearchLen; //×î³¤µÄ¿ÉÒÔ½øĞĞ·Ö´ÊµÄ³¤¶È
	int  m_iMaxSplitNum;             //×î¶àÄÜ·Ö³É¶àÉÙ¸ö´Ê
	string  m_strTmpSuffix; //ÁÙÊ±ÎÄ¼şµÄºó×º
};

#endif
