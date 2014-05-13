//this file is gbk encode
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <fstream>
#include <sys/time.h>

#include "util/XHStrUtils.h"
using namespace std;

#define KEY_SERVER_IP   "sip"
#define KEY_SERVER_PORT "sprt"
#define KEY_CGI_HELP    "help"
#define KEY_OUT_CODE    "output"
#define KEY_OUT_TXT_FORMAT "txt"
#define KEY_OUT_CB_CONTEXT "cb"
#define KEY_QUERY_HEAD "w" //查询词的tag
#define KEY_IN_ENCODE  "incode" //传入编码
#define KEY_OUT_ENCODE "outcode" //输出编码

#define DEFAULT_SERVER_IP   "127.0.0.1"
#define DEFAULT_SERVER_PORT "31415"
#define DEFAULT_OUT_CODE    "xml"

string strOutCode = "xml";
string strCB = "";
bool bOutUtf8 = false;
char szEncodeIn[1024];
char szEncodeOut[1024];

void LogExit(const char *str, bool bFromHelp = false)
{
	cout << "Content-type: text/plain;charset=gbk\n\n";
    //cout << "Content-type:text/plain\n\n";
    //cout << "<?xml version='1.0' encoding='gbk' ?><responses code='200' " << "time='" << (startt.tv_sec * 1000000 + startt.tv_usec) - starttime << "us' msg='good'>";
	if(strCB != "") {
		cout << "jQuery" << strCB << "&&" << "jQuery" << strCB << "(";
	}
	cout << "info:{" << endl;
	cout << "error:\"" << str << "\"" << endl;
	cout << "}" << endl;

	if(strCB != "") {
		cout << ")";
	}
	exit(0);
	/*
    cout << "Content-type:text/xml\n\n";
    if(bFromHelp) {
        cout << "<?xml version='1.0' encoding='gbk' ?><responses code='200' msg='help'>";
    } else {
        cout << "<?xml version='1.0' encoding='gbk' ?><responses code='200' msg='fault'>";
    }
    cout << "\n" << str << "\n";
    cout << "</responses>\n";
    //str = "<?xml version='1.0' encoding='utf-8' ?><responses code='200' msg='fault'></responses>";
    //cout<<str<<endl;
    //这里出错的时候返回的也是空
    exit(0);
	*/
}

void CgiHelp(const string & strInfo, bool bFromHelp = false)
{
    string strHelp = "\n<info>" + strInfo + "</info>\n";
    strHelp += "<response>output 输出编码,默认为:" + string(DEFAULT_OUT_CODE) + "(xml/json先仅xml)</response>\n";
    strHelp += "<response>sip 服务器的ip,默认为:" + string(DEFAULT_SERVER_IP) + "</response>\n";
    strHelp += "<response>sprt 服务器的端口号,默认为:" + string(DEFAULT_SERVER_PORT) + "</response>\n";
    strHelp += "<response>txt output html</response>\n";
    strHelp += "<response>help 只要有help则是显示帮助.</response>\n";
    LogExit(strHelp.c_str(), bFromHelp);
}

int StrTokenize(vector<string> &result, const string& src, string tok, bool trim = false, string null_subst = " ")
{
    int num = 0;
    if ( src.empty()) {
        return num;
    }
    if (tok.empty() ) {
        result.push_back(src);
        num = 1;
        return num;
    }

    int pre_index = 0, index = 0, len = 0;
    while ( (index = src.find_first_of(tok, pre_index)) != string::npos ) {
        if ( (len = index-pre_index) != 0) {
            result.push_back(src.substr(pre_index, len));
        } else if ( !trim ) {
            result.push_back(null_subst);
        }
        ++num;
        pre_index = index+1;
    }

    string endstr = src.substr(pre_index);
    if ( !trim ) {
        result.push_back( endstr.empty()?null_subst:endstr );
    } else if ( !endstr.empty() ) {
        result.push_back(endstr);
    }

    return ++num;
}
/*
the file to recieve cgi and search result return ;
test the multi-thread program;
*/
inline ssize_t safe_read(int fd, void* buf, size_t size)
{
    ssize_t cc;
    size_t sz = size;
    char* bp =(char *) buf;
    do {
        cc = read(fd, bp, sz);
        if ( cc > 0 ) {
            bp += cc;
            sz -= cc;
        } else if ( cc==0 )	// EOF
            return (sz) ? (size-sz) : 0;
        else if ( cc < 0 && errno != EINTR && errno != EAGAIN)
            return cc;
    }
    while (sz > 0);
    return size;
}

inline ssize_t safe_write(int fd, const void* buf, size_t size)
{
    ssize_t cc;
    size_t sz = size;
    const char *bp = (const char *)buf;
    if ( size==0 )	return 0;
    do {
        cc = write(fd, bp, sz);
        if ( cc > 0 ) {
            bp += cc;
            sz -= cc;
        } else if ( cc < 0 && errno != EINTR && errno != EAGAIN)
            return cc;
    }
    while (sz > 0);
    return size;
}



int htoi(char *s)
{
    const char *digits = "0123456789ABCDEF";

    if (islower(s[0])) s[0]=toupper(s[0]);
    if (islower(s[1])) s[1]=toupper(s[1]);

    return 16 * (strchr(digits, s[0])-strchr(digits,'0')) +(strchr(digits,s[1])-strchr(digits,'0'));
}


string ConvertString(const char *str)
{
    size_t len = strlen(str);
    string s;
    if(len > 150)
         return "";
    size_t pos = 0;
    while (pos < len) {
        if (str[pos] == '%' && pos < len-2) {
            char t[3];
            t[0] = str[pos+1];
            t[1] = str[pos+2];
            t[2] = 0;
            char c = htoi(t);
            s.push_back(c);
            pos+=2;
        } else if (str[pos] == '+') {
            s.push_back(' ');
        } else {
            s.push_back(str[pos]);
        }
        pos++;
    }
    return s;
}

string GetQueryString()
{
    string strRet = "";
    char *szMethord = getenv("REQUEST_METHOD");
    if(!szMethord) {
        CgiHelp("no REQUEST_METHOD");
    }
    if(strcmp(szMethord, "POST") == 0) {
        size_t iLen = atoi(getenv("CONTENT_LENGTH"));
        char szArrBuf[iLen + 1];
        szArrBuf[iLen]= '\0';
        for (size_t iLoop = 0; iLoop < iLen; iLoop++) {
            szArrBuf[iLoop] = getchar();
            if (szArrBuf[iLoop] == ',') szArrBuf[iLoop] = ';';
        }
        fflush(stdin);
        strRet = szArrBuf;
    } else if(strcmp(szMethord, "GET") == 0) {
        strRet = getenv("QUERY_STRING");
    } else {
        CgiHelp("not get/post");
    }
    return strRet;
}

void GetQueryRes(const string & strUrlQuery, const string & strIp, const int & iPort)
{
	timeval startt;
	gettimeofday(&startt,0);
	long starttime = (startt.tv_sec * 1000000 + startt.tv_usec);
    int sockfd;
    struct hostent *host;
    struct sockaddr_in serv_addr;
    host = gethostbyname(strIp.c_str());
    char buftop[ 21 ];
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(iPort);
    serv_addr.sin_addr = *(( struct in_addr *)host->h_addr );
    bzero( &(serv_addr.sin_zero), 8 );
    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    int ret = connect( sockfd, (struct sockaddr *)&serv_addr, sizeof( struct sockaddr) ) ;
    if (ret == -1) LogExit("connect error");
    char head[12];
    *(int*)head = 0xFFFFEEEE;
    *(int*)(head+4) = strUrlQuery.size();
        
    char szSend[1000];

    if(strUrlQuery.size() >= sizeof(szSend) - 10) {
        LogExit("query is too long");
    }
    for (int i=0; i<8; i++)
        szSend[i] = head[i];
    for (int i=0; i<strUrlQuery.size(); i++)
         szSend[8+i] = strUrlQuery[i];
    
    ret = safe_write( sockfd, szSend, strUrlQuery.size()+8);
    if (ret == -1) LogExit("write error");
    memset( buftop, '\0',  21 );
    ret = safe_read( sockfd, buftop, 8);
    if (ret < 8)
    {
        LogExit("recv head error");
    }
    int size = *(int *)(buftop+4);
    //	cout << size << endl;
    char result[size+1];
    result[size] = '\0';
    ret = safe_read( sockfd, result, size);
    if (ret < (int)size)
    {
        LogExit("recv error");
    }
	gettimeofday(&startt,0);
    //cout<<"Content-type:text/xml\n\n";
	if(bOutUtf8) {
		cout << "Content-type: text/plain;charset=utf-8\n\n";
	} else {
		cout << "Content-type: text/plain;charset=gbk\n\n";
	}
    //cout << "Content-type:text/plain\n\n";
    //cout << "<?xml version='1.0' encoding='gbk' ?><responses code='200' " << "time='" << (startt.tv_sec * 1000000 + startt.tv_usec) - starttime << "us' msg='good'>";
	if(strCB != "") {
		cout << "jQuery" << strCB << "&&" << "jQuery" << strCB << "(";
	}
	cout << "{"<<endl;
	cout << "\"time\":\"" << (startt.tv_sec * 1000000 + startt.tv_usec) - starttime << "us\"," << endl;
	cout << "\"msg\":\"good\"," << endl;
	cout << "\"info\":{" << endl;
	if(bOutUtf8) {
		string strResult = result;
		XHStrUtils::strFormatConverse(strResult, "gbk", "utf-8");
		cout << strResult <<endl;
	} else {
		cout<<result<<endl;
	}
    close(sockfd);
	/*
    char *p = strstr(result, "<count>");
    char *p2 = strstr(result, "<searchtime>");
    char buf[1000];
    if (p == NULL)
    {
        strcpy(buf, "ok\t");
    }
    else
    {
        size_t len = (strstr(result, "</count>")-p)-7;
        memcpy(buf, p+7, len);
        buf[len] = '\t';
        buf[len+1] = 0;
    }
    if (p2 == NULL)
    {
        strcat(buf, "ok");
    }
    else
    {
        size_t len = (strstr(result, "</searchtime>")-p)-12;
        memcpy(buf, p+12, len);
        buf[len] = 0;
    }
	*/
	cout << "}\n}";
	if(strCB != "") {
		cout << ")";
	}
}

void SpecialParamAnly(const vector<string> & vecKey, string & strIp, int & iPort, string &strExtra)
{
    strIp = DEFAULT_SERVER_IP;
    iPort = atoi(DEFAULT_SERVER_PORT);
    string query_type;
    bool bHelp = false;
	bool bTxt  = false;
	bool bInUtf8 = false;
	string strQueryW = "";
	int iIdxQueryW = -1;
    for (size_t i=0; i<vecKey.size(); i++)
    {
        size_t iEqualPos = vecKey[i].find("=");
        if (iEqualPos > vecKey[i].size()) { //found
            continue;
        }
        iEqualPos++;

        if(strncasecmp(vecKey[i].c_str(), KEY_SERVER_IP, iEqualPos - 1) == 0 && strlen(KEY_SERVER_IP) == iEqualPos - 1) {
            strIp = vecKey[i].c_str()+iEqualPos;
        } else if(strncasecmp(vecKey[i].c_str(), KEY_SERVER_PORT, iEqualPos - 1) == 0 && strlen(KEY_SERVER_PORT) == iEqualPos - 1) {
            iPort = atoi(vecKey[i].c_str()+iEqualPos);
        } else if(strncasecmp(vecKey[i].c_str(), KEY_CGI_HELP, iEqualPos - 1) == 0 && strlen(KEY_CGI_HELP) == iEqualPos - 1) {
            bHelp = true;
        } else if(strncasecmp(vecKey[i].c_str(), KEY_OUT_CODE, iEqualPos - 1) == 0 && strlen(KEY_OUT_CODE) == iEqualPos - 1) {
        } else if(strncasecmp(vecKey[i].c_str(), KEY_OUT_TXT_FORMAT, iEqualPos - 1) == 0 && strlen(KEY_OUT_TXT_FORMAT) == iEqualPos - 1) {
			bTxt = atoi(vecKey[i].c_str()+iEqualPos);
        } else if(strncasecmp(vecKey[i].c_str(), KEY_IN_ENCODE, iEqualPos - 1) == 0 && strlen(KEY_IN_ENCODE) == iEqualPos - 1) {
			string strTmp = vecKey[i].c_str()+iEqualPos;
			if(strTmp == "utf-8" || strTmp == "utf8") {
				bInUtf8 = true;
			}
        } else if(strncasecmp(vecKey[i].c_str(), KEY_OUT_ENCODE, iEqualPos - 1) == 0 && strlen(KEY_OUT_ENCODE) == iEqualPos - 1) {
			string strTmp = vecKey[i].c_str()+iEqualPos;
			if(strTmp == "utf-8" || strTmp == "utf8") {
				bOutUtf8 = true;
			}
        } else if(strncasecmp(vecKey[i].c_str(), KEY_QUERY_HEAD, iEqualPos - 1) == 0 && strlen(KEY_QUERY_HEAD) == iEqualPos - 1) {
			strQueryW = vecKey[i].c_str() + iEqualPos;
        } else if(strncasecmp(vecKey[i].c_str(), KEY_OUT_CB_CONTEXT, iEqualPos - 1) == 0 && strlen(KEY_OUT_CB_CONTEXT) == iEqualPos - 1) {
			strCB = vecKey[i].c_str()+iEqualPos;
        }
    }
	if(bInUtf8 && strQueryW != "") {
		XHStrUtils::strFormatConverse(strQueryW, "utf-8", "gbk");
		if(strQueryW != "") {
			strExtra = "&w=" + strQueryW;
		}
	}
	if(bTxt) {
		cout << "Content-type:text/html\n\n";
	}
    if(bHelp) {
        CgiHelp("just help", true);
    }
}

int main( int argc, char **argv )
{
    vector<string> vecQuery;
    string strIp;
    int iPort;
    string strKey = "";
    string strUrl;
	if(argc == 2) {
		strKey = argv[1];
	} else {
		strKey = GetQueryString();
	}
	string strExtra = "";
    strUrl = ConvertString(strKey.c_str());
    StrTokenize(vecQuery, strUrl, "&");
    SpecialParamAnly(vecQuery, strIp, iPort, strExtra); //特殊参数的解析
	if(strExtra != "") strUrl += strExtra;
	GetQueryRes(strUrl, strIp, iPort);
	return 0;
}

