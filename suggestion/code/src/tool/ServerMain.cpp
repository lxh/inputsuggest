#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <cstring>
#include "util/SysConfigSet.h"
#include "isserver/ServerPlat.h"

static int s_iFdLog = -1;

void UsageAndExit(const char * pzProc)
{
	printf("start IS server file\n");
	printf("Usage: %s -c config.ini\n", pzProc);
	printf("Usage: %s -t taskname -c config.ini\n", pzProc);
	printf("taskname  ->can don't have(if don't have -t, then deal all the task of config.ini setting)\n");
	printf("config.ini->configure file path\n");
	exit(-1);
}


void writefile(const char * pzCxt)
{
    if(pzCxt == NULL) {
        return ;
    }
	if(s_iFdLog > 0) return;
    char szCxt[100];
    time_t t;
    tm *local;
    t = time(NULL);
    local = localtime(&t);
    snprintf(szCxt, sizeof(szCxt), "Local time is: %s:", asctime(local));
    write(s_iFdLog, szCxt, strlen(szCxt));
    write(s_iFdLog, pzCxt, strlen(pzCxt));
    write(s_iFdLog, "\n",  strlen("\n"));
}
void closefile()
{
    if(s_iFdLog > 0) {
        close(s_iFdLog);
        s_iFdLog = -1;
    }
}
void sig_exit(int sig)
{
    char szCxt[100];
    const char *sigstr[20]={"", "SIGHUP", "SIGINT", "SIGQUIT", "SIGILL", "", "SIGABRT", "", "SIGFPE", "SIGKILL", "", "SIGSEGV", "", "SIGPIPE", "SIGALRM","SIGTERM"};
    cerr<<"exit by signal "<<sigstr[sig]<<endl;

    snprintf(szCxt, sizeof(szCxt), "exit by signal %s", sigstr[sig]);
    writefile(szCxt);
    closefile();
    exit(1);
}

void sig_init()
{
    signal(SIGINT, sig_exit); 
    signal(SIGQUIT, sig_exit);
    signal(SIGABRT, sig_exit);
    signal(SIGTERM, sig_exit);
    signal(SIGILL, SIG_IGN);
    signal(SIGSEGV, sig_exit);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGKILL, sig_exit);
}
//如果是一个文件，返回1
static int IsGoodFile(char *szPath)
{
    struct stat fileStat;
    if(szPath == NULL) {
        return 0;
    }
    if(stat(szPath, &fileStat) < 0)
    { 
        return 0;
    }
    if(S_ISREG(fileStat.st_mode))
    {
        return 1;
    }
    return 0;
}
void Run(const char * procName, map<const char, string> mapPara)
{
	string strTaskName = "";
	string strConfigName = "";
	map<const char, string>::iterator iIterMap;
	iIterMap = mapPara.find('c');
	if(iIterMap == mapPara.end()) {
		UsageAndExit(procName);
	}
	strConfigName = iIterMap->second;
	CSysConfigSet::GetInstance(strConfigName);

	//判断是否需要记录日志
    int iWithLog = 0; //CSysConfigSet::GetInstance().GetInt(PARA_CONFIG_useserverlog);
	if(iWithLog) {
		string strLog = procName;
		strLog += ".lxh.log";
		s_iFdLog = open(strLog.c_str(), O_RDWR | O_CREAT | O_APPEND, 0644);
		if(s_iFdLog < 0) {
			printf("open file error:%s[tipserverlog]\n", procName);
			exit(-1);
		}
		string strLogCxt = "log path is " + strLog;
		writefile(strLogCxt.c_str());
	}
}
int main(int argc,char** argv)
{
	map<const char, string> mapPara;
	int iLoop;
	for(iLoop = 1; iLoop < argc - 1; iLoop+=2) {
		if(argv[iLoop][0] == '-') {
			mapPara[argv[iLoop][1]] = argv[iLoop + 1];
		} else {
			UsageAndExit(argv[0]);
		}
	}
    //if(IsGoodFile(argv[1]) != 1) {
    //    UsageAndExit(argv[0]);
    //}

	Run(argv[0], mapPara);

    writefile("begin");
    CServerPlat cSP;
    cSP.Run();
    closefile();
    return 0;
}

