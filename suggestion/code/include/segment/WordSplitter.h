#ifndef WORD_SPLITTER_H__
#define WORD_SPLITTER_H__

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

#include "segment/WordSplit.h"

using namespace std;

enum enumWordSplitType
{
	WST_PHRASE,
	WST_BASIC,
	WST_MIXED,

	WST_COUNT
};

// 1,Other 2,Road 3,POI 4,Type 5,Station 6,区县乡镇 7,City 19,数字 20,数字短语
enum enumWordType
{
	WT_OTHER      = 1,
	WT_ROAD       = 2,
	WT_POI        = 3,
	WT_TYPE       = 4,
	WT_STATION    = 5,
	WT_PLACENAME  = 6,
	WT_CITY       = 7,
	WT_DIGITAL    = 19,
	WT_DIGITALP   = 20
};

const string WORD_FIELDS_SPLITS_TAG = "/";
const string WORD_CONTENT_ATTRIBUTES_PREFFIX = "[";
const string WORD_CONTENT_ATTRIBUTES_SURFFIX = "]";
const string WORD_CONTENT_ATTRIBUTES_SPLITS_TAG = "-";

const size_t MAX_WORD_FREQUECE = 10000;

//[0-4]北京/7
class Word
{
public:
	Word(const string & str);

	friend ostream & operator <<(ostream & out, const Word & word );
	//friend istream & operator >>(istream & in, Word & word );
	void parse(const string& str);

	static bool isWordStr(const string& str);

	size_t sttindex;         //起始下标，从0开始计数，基本切分是以字符个数，其余切分参照基本切分
	size_t offset;        //偏移量
	size_t wordtype;        //词性
	size_t freq;            //词频 0 - 10000
    size_t termid;
	string word;			//词


};

/*
 * please input a string:
 * 北京市大恒科技大厦
 * Split: 
 * [0-2]北京市/7 [2-3]大恒科技大厦/3 
 * Basic: 
 * [0-4]北京/7 [4-2]市/1 [6-4]大恒/1 [10-4]科技/1 [14-4]大厦/3 
 * Phrase:
 * [0-2]北京市/7 [2-2]大恒科技/1 [2-3]大恒科技大厦/3 [3-2]科技大厦/5 
 *
 */
class WordSplitsResult
{
public:
	WordSplitsResult();
	void process(string  mixed = "", string basic = "", string phrase = "");
	
	//得到基本切分在短语切分中的位置
	int getBasicWordInPhraseIndex(size_t basic_index);
	void clear();

	friend ostream & operator <<(ostream & out, const WordSplitsResult & wsr);

    static void strTokenize(vector<string>&v,const string& src, string tok, bool trim=false, string null_subst = "");
    static vector<string> strTokenizeStr(const string& src, string tok, bool trim=false, string null_subst = "");

private:
	void str2Word(const string & str, vector<Word>&word);

public:
	vector<Word> mixedWords;   //混合切分
	vector<Word> basicWords;   //基本切分
	vector<Word> phraseWords;  //短语切分

};


class WordSplitter
{
public:
	~WordSplitter();

	//由于需要传入参数，请先期调该接口，完成预先初始化
	static int initial(const string& _dictdir);  

  static WordSplitter& getInstance();
 
	//调用CWordSplit接口
	CWordSplit* get()
	{
		return splitter;
	}

	//分词程序
	bool split(const string & input, WordSplitsResult &wsr);
private:
	WordSplitter(){};

    CWordSplit *splitter;

	static WordSplitter ws;
};


#endif //WORD_SPLITTER_H__

