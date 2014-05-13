#ifndef _WORD_STRING_INFO_H_
#define _WORD_STRING_INFO_H_
#include <string>
#include <vector>
#include <map>
#include "util/ReadBigFile.h"
#include "util/WriteBigFile.h"

//扩展信息设置在字符串的内部还是外部
#define EXTRA_INFO_SET_INSIDE  0 //内部->只有汉字词典
#define EXTRA_INFO_SET_OUTSIDE 1

using namespace std;

class CWordStringInfo {
public:	
	int AnlyInfo(const string & strInFile, const string & strOutFile, const string & strOuFileExtra = "", const int & iType = EXTRA_INFO_SET_OUTSIDE);

	unsigned int GetWordPos(const string & strKey) ;
	unsigned int GetReverseWordPos(const string & strKey);
	void Release();
	void Reverse();
private:
	int Process();
	static bool PairLess(pair<string, unsigned int> v1, pair<string, unsigned int> v2) {
		return v1.first < v2.first;
	};

private:
	string m_strInFile;
	string m_strOutFile;
	string m_strOutFileExtra;
	int    m_iType;
	int    m_iWordField;
	map<const string, unsigned int>      m_mapWordPos;
	vector<pair<string, unsigned int> >  m_vecReverseSort;
	unsigned int  m_iReverseVecSize;
};


#endif
