#ifndef _WRITE_LOG_H_
#define _WRITE_LOG_H_
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
using namespace std;


class CWriteLog {
public:
	CWriteLog(string strFileName);
	static CWriteLog & GetInstance(string strFileName="");

	~CWriteLog();
	void CloseFile();
	void WriteLog(const string & str);
private:
	int m_iFileLog;
	char m_szTime[32];

};
#endif
