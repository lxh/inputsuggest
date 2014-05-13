//gbk
#ifndef _XH_STR_UTILS_H_
#define _XH_STR_UTILS_H_
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>

#ifdef USE_ICONV
#define NEED_ICONV 1
#else
#define NEED_ICONV 0
#endif

#if NEED_ICONV
#include <iconv.h>
//#include "util/iconv/iconv.h"
#endif

using namespace std;
#define CONVERTPOOLLEN 3*1024*1024

class XHStrUtils {
public: //字符串切分
	//split strIn which is csv format to vecOut
	//切分csv格式的文件
	//return -1 error; else ok
	static int SplitCsvString(const string & strIn, vector<string> & vecOut);
	//用制定字符进行字符串切分
	static void StrTokenize(vector<string>& v, const string& src, string tok, bool trim = false);
	//切分gbk编码的字符串(包含一些不是特别正规的汉字，如第二个字节大于0)
	static void StrTokenizeGBK(vector<string> &v, const string& src, string tok);
	//用指定的字符串切分
	static void StrTokenizeStr(vector<string>& v, const string& src, string tok, bool trim = false);

#if NEED_ICONV
	static int IconvConvert(const char* instr,int inlen, char* outstr, const char* inputformat, const char * outputformat);
	static void strFormatConverse(string &input, string inputFormat = "utf-8", string outputFormat = "gbk");
#endif
public: 
	//噪音处理
	static void FiltNoiseSymbol(string & str);
	//trim
	static void trim(string & s);
	static void trimleft(string & s);
	static void trimright(string & s);

	static void ToCase(string & s, bool bUp=true);

	static void JoinMultiSpace(string & s);

private:
	static bool isspace(char ch);
};


#endif
