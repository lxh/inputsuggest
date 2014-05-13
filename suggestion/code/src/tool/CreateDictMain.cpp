#include "isdict/CreateDict.h"
#include "util/XHStrUtils.h"
#include "util/SysConfigSet.h"

void UsageAndExit(const char * pzProc)
{
	printf("create dict IS file\n");
	printf("Usage: %s -c config.ini\n", pzProc);
	printf("Usage: %s -t taskname -c config.ini\n", pzProc);
	printf("taskname  ->can don't have(if don't have -t, then deal all the task of config.ini setting)\n");
	printf("config.ini->configure file path\n");
	exit(-1);
}

void Run(const char * pzProc, map<const char, string> & mapPara)
{
	string strTaskName = "";
	string strConfigName = "";
	map<const char, string>::iterator iIterMap;
	iIterMap = mapPara.find('c');
	if(iIterMap == mapPara.end()) {
		UsageAndExit(pzProc);
	}
	strConfigName = iIterMap->second;
	CSysConfigSet clsSysConfigSet;
	clsSysConfigSet.ReadConfig(strConfigName);

	iIterMap = mapPara.find('t');
	if(iIterMap != mapPara.end()) {
		strTaskName = iIterMap->second;
	} else {
		strTaskName = clsSysConfigSet.GetString(PARA_CONFIG_tasklist);
	}
	vector<string> vecSplit;
	XHStrUtils::StrTokenize(vecSplit, strTaskName, SPLIT_MULTI_TAKENAME_STR);

	int iLoop;
	for(iLoop = 0; iLoop < vecSplit.size(); iLoop++) {
		CCreateDict clsCD;
		clsCD.Run(vecSplit[iLoop], &clsSysConfigSet);
	}
}

int main(int argc, char ** argv)
{
	if(argc <= 2) {
		UsageAndExit(argv[0]);
	}

	map<const char, string> mapPara;
	int iLoop;
	for(iLoop = 1; iLoop < argc - 1; iLoop+=2) {
		if(argv[iLoop][0] == '-') {
			mapPara[argv[iLoop][1]] = argv[iLoop + 1];
		} else {
			UsageAndExit(argv[0]);
		}
	}
	Run(argv[0], mapPara);

	return 0;
}
