#include <stdio.h>
#include <stdlib.h>
#include "util/CompressPinyin.h"


int main(int argc, char **argv)
{
	CCompressPinyin a;
	char szInfo[20];
	int iSize = 3;
	int iLoop;
	bool bRet = a.CompressStr(iSize, "ZhiCunFingFa", szInfo);

	for(iLoop = 0; iLoop < iSize; iLoop++) {
		printf("ret:%d:%d:%d\n", bRet, iLoop, szInfo[iLoop]);
	}
	string strFull;
	bRet = a.UnCompressStr("mmm", szInfo, strFull);
	printf("ret:%d:%s\n", bRet, strFull.c_str());
	return 0;
}
