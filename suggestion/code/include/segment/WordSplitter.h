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

// 1,Other 2,Road 3,POI 4,Type 5,Station 6,�������� 7,City 19,���� 20,���ֶ���
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

//[0-4]����/7
class Word
{
public:
	Word(const string & str);

	friend ostream & operator <<(ostream & out, const Word & word );
	//friend istream & operator >>(istream & in, Word & word );
	void parse(const string& str);

	static bool isWordStr(const string& str);

	size_t sttindex;         //��ʼ�±꣬��0��ʼ�����������з������ַ������������зֲ��ջ����з�
	size_t offset;        //ƫ����
	size_t wordtype;        //����
	size_t freq;            //��Ƶ 0 - 10000
    size_t termid;
	string word;			//��


};

/*
 * please input a string:
 * �����д��Ƽ�����
 * Split: 
 * [0-2]������/7 [2-3]���Ƽ�����/3 
 * Basic: 
 * [0-4]����/7 [4-2]��/1 [6-4]���/1 [10-4]�Ƽ�/1 [14-4]����/3 
 * Phrase:
 * [0-2]������/7 [2-2]���Ƽ�/1 [2-3]���Ƽ�����/3 [3-2]�Ƽ�����/5 
 *
 */
class WordSplitsResult
{
public:
	WordSplitsResult();
	void process(string  mixed = "", string basic = "", string phrase = "");
	
	//�õ������з��ڶ����з��е�λ��
	int getBasicWordInPhraseIndex(size_t basic_index);
	void clear();

	friend ostream & operator <<(ostream & out, const WordSplitsResult & wsr);

    static void strTokenize(vector<string>&v,const string& src, string tok, bool trim=false, string null_subst = "");
    static vector<string> strTokenizeStr(const string& src, string tok, bool trim=false, string null_subst = "");

private:
	void str2Word(const string & str, vector<Word>&word);

public:
	vector<Word> mixedWords;   //����з�
	vector<Word> basicWords;   //�����з�
	vector<Word> phraseWords;  //�����з�

};


class WordSplitter
{
public:
	~WordSplitter();

	//������Ҫ��������������ڵ��ýӿڣ����Ԥ�ȳ�ʼ��
	static int initial(const string& _dictdir);  

  static WordSplitter& getInstance();
 
	//����CWordSplit�ӿ�
	CWordSplit* get()
	{
		return splitter;
	}

	//�ִʳ���
	bool split(const string & input, WordSplitsResult &wsr);
private:
	WordSplitter(){};

    CWordSplit *splitter;

	static WordSplitter ws;
};


#endif //WORD_SPLITTER_H__

