#ifndef _FILE_COMMON_H_
#define _FILE_COMMON_H_
#include <string>             
#include <map>                
#include "util/WriteBigFile.h"
#include "common/XHTreeStruct.h"

using namespace std;    


class CFileCommon {
public:
	static bool ReadOneFile(const string & strFileName, char *& pData, unsigned int & uiSize);
	static bool WordToLineNumber(const string & strFileName, const string & strSpliterChar, const int iWordField, map<const string, unsigned int> & mapWordLineId);
	static int GetFileLines(const string & strFileName);
	static void WriteOneFile(const string & strFileName, const char * pData, const unsigned int uiSize);
	static void WriteOneFileAppend(const string & strFileName, const char * pData, const unsigned int uiSize);
};

#endif
