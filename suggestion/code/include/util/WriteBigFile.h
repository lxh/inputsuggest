#ifndef _WRITE_BIG_FILE_H_
#define _WRITE_BIG_FILE_H_
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>

using namespace std;

class CWriteBigFile {
public:
	CWriteBigFile() {
		Init();
	};
	bool Open(const char szFileName[], const int iBuffSize = 40 * 1024 * 1024) {
		return Open2(szFileName, iBuffSize);
	};
	bool Write(const char *  pzBuf, int iLen) {
		return Write2(pzBuf, iLen);
	};
	bool WriteLine(const char * pzBuf) {
		int iLen = 0;
		iLen = strlen(pzBuf);
		Write2(pzBuf, iLen);
		static const char * p = "\n";
		Write2(p, 1);
	};
	~CWriteBigFile() {
		Release();
	};
private:
	void Init() {
		m_iWritePos = 0;
		m_pDataBuff = NULL;
		m_iFd = -1;
	};
	bool Open2(const char szFileName[], const int iBuffSize) {
		m_iBuffSize = iBuffSize;
		m_iFd = open(szFileName, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		if(m_iFd <= 0) {
			printf("open file error:%s %d[%s %d]\n", szFileName, m_iFd, __FILE__, __LINE__);
			return false;
		}
		m_pDataBuff = new char[m_iBuffSize];
		if(!m_pDataBuff) {
			printf("new memory error[%d][%s %d]\n", m_iBuffSize, __FILE__, __LINE__);
			exit(-1);
		}
		return true;
	};
	bool Write2(const char *  pzBuf, int iLen) {
		if(iLen > m_iBuffSize) {
			printf("the write string is too long[%d %d]\n", iLen, m_iBuffSize);
			return false;
		}
		if(iLen > m_iBuffSize - m_iWritePos) {
			write(m_iFd, m_pDataBuff, m_iWritePos);
			m_iWritePos = 0;
		}
		memcpy(m_pDataBuff + m_iWritePos, pzBuf, iLen);
		m_iWritePos += iLen;
		return true;
	};
	void Release() {
		if(m_iFd > 0) {
			if(m_pDataBuff) {
				write(m_iFd, m_pDataBuff, m_iWritePos);
			}
			close(m_iFd);
		}
		if(m_pDataBuff) {
			delete []m_pDataBuff;
			m_pDataBuff = NULL;
		}
	};
private:
	int    m_iFd;
	char * m_pDataBuff;

	int m_iBuffSize;
	int m_iWritePos; //当前读取到了哪个位置
};

#endif
