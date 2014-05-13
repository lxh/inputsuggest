#ifndef _READ_BIG_FILE_H_
#define _READ_BIG_FILE_H_
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>

using namespace std;

class CReadBigFile {
public:
	CReadBigFile() {
		Init();
	};
	bool Open(const char szFileName[], const int iBuffSize = 40 * 1024 * 1024) {
		return Open2(szFileName, iBuffSize);
	};
	bool GetLine(char * & pzBuf) {
		return GetLine2(pzBuf);
	};
	~CReadBigFile() {
		Release();
	};
private:
	void Init() {
		m_iReadPos = -1;
		m_iFd = -1;
		m_pDataBuff = NULL;
		m_iReadOutSize = 0;
		m_bLastBlock = false;
		m_bOver = false;
	};
	bool Open2(const char szFileName[], const int iBuffSize = 40 * 1024 * 1024) {
		m_iBuffSize = iBuffSize;
		m_iFd = open(szFileName, O_RDONLY);
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
	bool GetLine2(char * & pzBuf) {
		if(m_bOver) return false;
		if(m_iReadOutSize == m_iReadPos && m_iReadOutSize != m_iBuffSize) {
			return false;
		}
		if(m_iReadOutSize == 0) { //��һ�ζ�ȡ
			if(m_bLastBlock) {
				return false;
			}
			m_iReadOutSize = read(m_iFd, m_pDataBuff, m_iBuffSize);
			if(m_iReadOutSize != m_iBuffSize) {
				m_bLastBlock = true;
			}
			m_iReadPos = 0;
		}
		int iLoop;
		for(iLoop = m_iReadPos; iLoop < m_iReadOutSize; iLoop++) {
			if(m_pDataBuff[iLoop] == '\n') {
				m_pDataBuff[iLoop] = 0;
				if(iLoop > 1 && m_pDataBuff[iLoop - 1] == '\r') { // \r\n
					m_pDataBuff[iLoop - 1] = 0;
				}
				break;
			}
		}
		if(iLoop == m_iReadOutSize) { //û���ҵ�
			if(m_bLastBlock) {
				m_pDataBuff[m_iReadOutSize] = 0;
				m_bOver = true;
				pzBuf = m_pDataBuff + m_iReadPos;
				return true;
			}
			memcpy(m_pDataBuff, m_pDataBuff + m_iReadPos, m_iReadOutSize - m_iReadPos);
			//int jLoop;
			//for(jLoop = 0; jLoop < m_iReadOutSize - m_iReadPos; jLoop++) {
			//}
			m_iReadPos = m_iReadOutSize - m_iReadPos;
			int iReadTmp = read(m_iFd, m_pDataBuff + m_iReadPos, m_iBuffSize - m_iReadPos);
			if(iReadTmp != m_iBuffSize - m_iReadPos) {
				m_bLastBlock = true;
			}
			m_iReadOutSize = m_iReadPos + iReadTmp;
			for(iLoop = m_iReadPos; iLoop < m_iReadOutSize; iLoop++) {
				if(m_pDataBuff[iLoop] == '\n') {
					m_pDataBuff[iLoop] = 0;
					if(iLoop > 1 && m_pDataBuff[iLoop - 1] == '\r') { // \r\n
						m_pDataBuff[iLoop - 1] = 0;
					}
					break;
				}
			}
			if(iLoop == m_iReadOutSize) { //û���ҵ�
				if(!m_bLastBlock) {
					int iS = 64;
					iS = 64 > m_iBuffSize ? m_iBuffSize : 64;
					m_pDataBuff[iS] = 0;
					printf("big line:%s[%s %d]\n", m_pDataBuff, __FILE__, __LINE__);
					exit(-1);
				}
				m_bOver = true;
				m_pDataBuff[m_iReadOutSize] = 0;
			} else {
				m_iReadPos = iLoop + 1;
			}
			pzBuf = m_pDataBuff;
			return true;
		}
		pzBuf = m_pDataBuff + m_iReadPos;
		m_iReadPos = iLoop + 1;
		return true;
	};
	void Release() {
		if(m_pDataBuff) {
			delete []m_pDataBuff;
			m_pDataBuff = NULL;
		}
		if(m_iFd > 0) {
			close(m_iFd);
		}
	};
private:
	int    m_iFd;
	char * m_pDataBuff;

	bool m_bLastBlock; //���һ�ζ�ȡ
	bool m_bOver;      //����
	int m_iBuffSize;
	int m_iReadOutSize; //��ǰ��ȡ���˶�������
	int m_iReadPos; //��ǰ��ȡ�����ĸ�λ��
};

#endif
