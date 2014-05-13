#include <unistd.h>
#include <vector>             
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "util/FileCommon.h"
#include "util/ReadBigFile.h"
#include "util/XHStrUtils.h"

//��һ���ļ�,���ļ���word��Ӧ�����ļ��еı�ŷ���
//iWordField->�±��0��ʼ
//����ж��word�ظ�,���и��Ǵ���
bool CFileCommon::WordToLineNumber(const string & strFileName, const string & strSpliterChar, const int iWordField, map<const string, unsigned int> & mapWordLineId)
{
	CReadBigFile fdR;
	fdR.Open(strFileName.c_str());
	char *p;
	int iFileLines = 0;
	while(fdR.GetLine(p)) {
		vector<string> vecSplit;
		XHStrUtils::StrTokenizeGBK(vecSplit, p, strSpliterChar);
		if(vecSplit.size() < iWordField + 1) {
			printf("error:(exit)file:%s, word to line(%s)[%s %d]\n", strFileName.c_str(), p, __FILE__, __LINE__);
			exit(-1);
			//��ʽ�汾,����Ӧ�÷���false
			return false;
		}
		mapWordLineId[vecSplit[iWordField]] = iFileLines;
		iFileLines++;
	}
	return true;
}

int CFileCommon::GetFileLines(const string & strFileName)
{
	CReadBigFile fdR;
	fdR.Open(strFileName.c_str());
	char *p;
	int iFileLines = 0;
	while(fdR.GetLine(p)) {
		iFileLines++;
	}
	return iFileLines;
}

//ǰ�ĸ��ֽ�Ϊ�ļ����ݵĴ�С����Сֵ����ǰ�ĸ��ֽڣ�
void CFileCommon::WriteOneFile(const string & strFileName, const char * pData, const unsigned int uiSize)
{
	int iFd = open(strFileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

	unsigned int uiTotal = uiSize + 4;
	write(iFd, &uiTotal, 4);
	write(iFd, (const char *)pData, uiSize);
	
	close(iFd);
}
//ǰ�ĸ��ֽ�Ϊ�ļ����ݵĴ�С����Сֵ����ǰ�ĸ��ֽڣ�
void CFileCommon::WriteOneFileAppend(const string & strFileName, const char * pData, const unsigned int uiSize)
{
	int iFd = open(strFileName.c_str(), O_RDWR, S_IRUSR | S_IWUSR);

	unsigned int uiHead = 0;
	int iRetSize = lseek(iFd, 0, SEEK_SET);
	if((iRetSize = read(iFd, &uiHead, 4)) != 4) {
		printf("error:(exit) read file %s error(%d:%d)(fd:%d; errno:%d %s)[%s %d]\n", strFileName.c_str(), iRetSize, 4, iFd, errno, strerror(errno), __FILE__, __LINE__);
		close(iFd);
		exit(-1);
	}
	uiHead += uiSize;
	iRetSize = lseek(iFd, 0, SEEK_SET);
	if((iRetSize = write(iFd, &uiHead, 4)) != 4) {
		printf("error:(exit) write file %s error(%d:%d)[%s %d]\n", strFileName.c_str(), iRetSize, 4, __FILE__, __LINE__);
		close(iFd);
		exit(-1);
	}
	uiHead += uiSize;
	iRetSize = lseek(iFd, 0, SEEK_END);

	if((iRetSize = write(iFd, (const char *)pData, uiSize)) != uiSize) {
		printf("error:(exit) write file %s error(%d:%d)[%s %d]\n", strFileName.c_str(), iRetSize, uiSize, __FILE__, __LINE__);
		close(iFd);
		exit(-1);
	}
	uiHead += uiSize;
	
	close(iFd);
}
