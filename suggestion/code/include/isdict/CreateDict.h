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
	vector<string> vecIds; //对应的汉字下标
	vector<string> vecExtra2; //扩展信息�
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
	CSysConfigSet * m_pSysConfigSet; //配置文件的信息
	string          m_strTaskName;   //任务的名称
	string          m_strDataPath;   //数据的路径
	bool            m_bDictNeedCreate[DICT_MAX_SIZE]; //需要处理的词典
	bool            m_bNeedPinyinDeal; //是否含有拼音的处理（如果没有拼音，那么就不需要获得拼音)

	//关于分词的几个参数
	int  m_iMinSplitCenterWordLen;   //中间查询的词最短长度为多少
	int  m_iMinSplitCenterSearchLen; //最短的可以进行分词的长度
	int  m_iMaxSplitCenterSearchLen; //最长的可以进行分词的长度
	int  m_iMaxSplitNum;             //最多能分成多少个词
	string  m_strTmpSuffix; //临时文件的后缀
};

#endif
