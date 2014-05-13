#include "segment/WordSplitter.h"

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
////////////////    class Word       //////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

Word::Word(const string & str)
{
	sttindex = 0;
	offset = 0;
	wordtype = WT_OTHER;
	freq = 0;

	parse(str);
}

bool Word::isWordStr(const string& str)
{
	size_t pos1 = str.find( WORD_CONTENT_ATTRIBUTES_PREFFIX );
	size_t pos2 = str.find(WORD_CONTENT_ATTRIBUTES_SPLITS_TAG);
	size_t pos3 = str.find(WORD_CONTENT_ATTRIBUTES_SURFFIX);
	size_t pos4 = str.rfind( WORD_FIELDS_SPLITS_TAG );

	//[0-4]北京/7/123456/107
	return (  pos4 < str.size() && 
		(pos3 < pos4 ) && 
		(pos2 < pos3 && isdigit(str[pos2+1]))&& 
		(pos1 < pos2 && pos1 == 0)
		);
}

ostream & operator <<(ostream & out, const Word & word)
{
	out<<WORD_CONTENT_ATTRIBUTES_PREFFIX;
    out<<word.sttindex;
    out<<WORD_CONTENT_ATTRIBUTES_SPLITS_TAG;
    out<<word.offset;
    out<<WORD_CONTENT_ATTRIBUTES_SURFFIX;
    out<<word.word;
    
    out<<WORD_FIELDS_SPLITS_TAG;
    out<<word.termid;

    out<<WORD_FIELDS_SPLITS_TAG;
    out<<word.freq;

    out<<WORD_FIELDS_SPLITS_TAG;
    out<<word.wordtype;

	return out;
}
void Word::parse(const string& str)
{
	if(!isWordStr(str))
		return;

	size_t stt = str.find(WORD_CONTENT_ATTRIBUTES_PREFFIX);
	size_t end = str.find( WORD_CONTENT_ATTRIBUTES_SURFFIX, stt);
	if(end < str.size() && stt < end)
	{
		stt+=1;
		size_t pos = str.find( WORD_CONTENT_ATTRIBUTES_SPLITS_TAG, stt);
		if(pos < end)
		{
			sttindex = atoi(str.substr(stt, pos-stt).c_str());
			pos+=1;
			offset = atoi(str.substr(pos, end - pos).c_str());
		}

		stt = end+1;
	}
	else
	{
		stt = 0;
		end = 0;
	}

    vector<string> splits;
    string tmp_str = str.substr( stt, str.size()-stt);
    WordSplitsResult::strTokenize(splits, tmp_str, WORD_FIELDS_SPLITS_TAG );
    if(splits.size() < 4 )
        return;

    size_t end_index = splits.size() - 1;
    wordtype = atoi(splits[end_index].c_str());
    freq = atoi(splits[end_index - 1].c_str() );
    termid = atoi( splits[end_index-2].c_str());

    word = "";
    for(size_t k = 0; k < end_index - 2; k++)
    {
        word += splits[k] + WORD_FIELDS_SPLITS_TAG;
    }
    if( !word.empty() )
        word = word.substr(0, word.size()-1);

	if(wordtype < WT_OTHER  || (wordtype > WT_CITY  && wordtype != WT_DIGITAL && wordtype != WT_DIGITALP))
		wordtype = WT_OTHER;

}



///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//////////////// class WordSplitsResult  //////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

WordSplitsResult::WordSplitsResult()
{
}

void WordSplitsResult::clear()
{
	mixedWords.clear();
	basicWords.clear();
	phraseWords.clear();
}

void WordSplitsResult::strTokenize(vector<string>&v,const string& src, string tok, bool trim, string null_subst)
{
	if( src.empty())
	{
		return ;
	}
	if(tok.empty() )
	{
		v.push_back(src);
		return ;
	}

	size_t pre_index = 0, index = 0, len = 0;
	while( (index = src.find_first_of(tok, pre_index)) != string::npos )
	{
		if( (len = index-pre_index)!=0 )
			v.push_back(src.substr(pre_index, len));
		else if(trim==false)
			v.push_back(null_subst);
		pre_index = index+1;
	}
	string endstr = src.substr(pre_index);
	if( trim==false ) v.push_back( endstr.empty()?null_subst:endstr );
	else if( !endstr.empty() ) v.push_back(endstr);

	return ;

}

vector<string> WordSplitsResult::strTokenizeStr(const string& src, string tok, bool trim, string null_subst)
{
	vector<string> v;
	if( src.empty())
	{
		return v;
	}
	if(tok.empty() )
	{
		v.push_back(src);
		return v;
	}

	size_t pre_index = 0, index = 0, len = 0;
	while( (index = src.find(tok, pre_index)) != string::npos )
	{
		if( (len = index-pre_index)!=0 )
			v.push_back(src.substr(pre_index, len));
		else if(trim==false)
			v.push_back(null_subst);
		pre_index = index +	tok.size();
	}
	string endstr = src.substr(pre_index);
	if( trim==false ) v.push_back( endstr.empty()?null_subst:endstr );
	else if( !endstr.empty() ) v.push_back(endstr);
	return v;

}



void WordSplitsResult::str2Word(const string & str, vector<Word>& words)
{
	vector<string> splits;
	strTokenize(splits, str, " ", true, "");
	string tmp;
	for(size_t i = 0; i < splits.size(); i++ )
	{
		tmp = splits[i];
		if(!Word::isWordStr(tmp) && ++i < splits.size())
		{
			tmp += + " " + splits[i];
			//continue; //忽略空格
		}
		if( i >= splits.size())
			break;

		words.push_back(Word(tmp));
	}
}


void WordSplitsResult::process(string mixed, string basic, string phrase)
{
	str2Word(mixed, mixedWords);
	str2Word(basic, basicWords);
	str2Word(phrase, phraseWords);
}

ostream & operator <<(ostream & out, const WordSplitsResult & wsr)
{
	for (size_t i = 0; i < wsr.mixedWords.size(); i++)
		out<<wsr.mixedWords[i]<<" ";
	out<<endl;

	for (size_t i = 0; i < wsr.basicWords.size(); i++)
		out<<wsr.basicWords[i]<<" ";
	out<<endl;

	for (size_t i = 0; i < wsr.phraseWords.size(); i++)
		out<<wsr.phraseWords[i]<<" ";
	out<<endl;

	return out;
}

//根据
int WordSplitsResult::getBasicWordInPhraseIndex(size_t basic_index)
{
	if (basic_index >= basicWords.size())
		return -1;

	//针对存在的包含情况，取最短的分词
	size_t cur_phrase_len = 10000;
	int retv = -1;
	string basic_word = basicWords[basic_index].word;
	for (size_t i = 0; i < phraseWords.size(); i++)
	{
		if ( (basic_index >= phraseWords[i].sttindex && basic_index < phraseWords[i].sttindex + phraseWords[i].offset) && 
			  cur_phrase_len > phraseWords[i].word.size()
		   )
		{
			retv = static_cast<int>(i);
			cur_phrase_len = phraseWords[i].word.size();
		};
	}

	return retv;
}



///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//////////////// class WordSplitter  //////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

WordSplitter WordSplitter::ws;

WordSplitter::~WordSplitter()
{
	if (splitter != NULL)
    {
        delete splitter;
        splitter = NULL;
    }
}

int WordSplitter::initial(const string& dictdir)
{

	if (ws.splitter == NULL)
	{
		if (dictdir.empty())
		{
			cout<<"error: the dict dir is empty."<<endl;
			return -1;
		}

		ws.splitter = new CWordSplit(dictdir.c_str());
	}

	return 0;
}

WordSplitter& WordSplitter::getInstance()
{   
	return ws; 
}   

bool WordSplitter::split(const string &input, WordSplitsResult &wsr)
{
	const size_t length = input.size()*10 +1000;
	char *str[3];
	for (size_t i = 0; i < 3; i++)
		str[i] = new char[length];
	
	splitter->funSplitString(input.c_str(), str[0], str[1], str[2], true, true,true,true);

	wsr.clear();
	wsr.process(str[0], str[1], str[2]);

	for (size_t i = 0; i < 3; i++)
		delete [](str[i]);

	return true;
	
}




#if 0
//test

int main()
{

	string line;
	ifstream infile("koubei.name");
	ofstream outputfile("output.txt", ios::trunc);

	WordSplitsResult wsr;
	int count = 0;
	while(getline(infile, line))
	{
		outputfile<<line<<endl;

		if(++count % 1000 == 0)
			cout<<count<<endl;
		//调用wsr的split（分词程序）
		WordSplitter::getInstance().split(line, wsr);

		outputfile<<wsr<<endl;
	}

	infile.close();
	outputfile.close();
	cout<<endl;
	
	return 1;
}

#endif

