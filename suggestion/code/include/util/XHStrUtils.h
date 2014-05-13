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
public: //�ַ����з�
	//split strIn which is csv format to vecOut
	//�з�csv��ʽ���ļ�
	//return -1 error; else ok
	static int SplitCsvString(const string & strIn, vector<string> & vecOut);
	//���ƶ��ַ������ַ����з�
	static void StrTokenize(vector<string>& v, const string& src, string tok, bool trim = false);
	//�з�gbk������ַ���(����һЩ�����ر�����ĺ��֣���ڶ����ֽڴ���0)
	static void StrTokenizeGBK(vector<string> &v, const string& src, string tok);
	//��ָ�����ַ����з�
	static void StrTokenizeStr(vector<string>& v, const string& src, string tok, bool trim = false);

#if NEED_ICONV
	static int IconvConvert(const char* instr,int inlen, char* outstr, const char* inputformat, const char * outputformat);
	static void strFormatConverse(string &input, string inputFormat = "utf-8", string outputFormat = "gbk");
#endif
public: 
	//��������
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
