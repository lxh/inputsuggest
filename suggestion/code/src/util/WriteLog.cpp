#include "util/WriteLog.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

CWriteLog::CWriteLog(string strFileName)
{
	if(strFileName == "") {
		m_iFileLog = -1;
	} else {
		m_iFileLog = open(strFileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	}
}

CWriteLog & CWriteLog::GetInstance(string strFileName)
{
	static CWriteLog log(strFileName);
	return log;
}


CWriteLog::~CWriteLog()
{
	CloseFile();
}

void CWriteLog::CloseFile()
{
	if(m_iFileLog != -1) {
	    close(m_iFileLog);
		m_iFileLog = -1;
	}
}
void CWriteLog::WriteLog(const string & str)
{
	if(m_iFileLog != -1) {
		timeval startt;
		gettimeofday(&startt,0);
		long starttime = (startt.tv_sec + startt.tv_usec / 1000000); //s
		//long starttime = (startt.tv_sec * 1000000 + startt.tv_usec); //us
		string strLog = str + "\n";
		snprintf(m_szTime, sizeof(m_szTime), "%ld-->", starttime);
		strLog = m_szTime + strLog;
		write(m_iFileLog, strLog.c_str(), strLog.size());
	}
}
