#include "netcommon/NetServer.h"
#include "util/Pinyin.h"
#include "util/SysConfigSet.h"
#include "isserver/ServerPlat.h"
#include "isserver/InputTip.h"

using namespace std;
class TipDataUpdateThread: public CRunnalble
{
public:
    virtual void run()
    {
		//不断检查新的索引要更新
        CInputTip::GetInstance().UpdateThread();
    }
};

class QuerySearchThread : public CRunnalble
{
public:
    virtual void run()
    {
        LockedQueue<QuerySearchTask> &lq = NetServer::getQuerySearchTaskQueue();
        LockedQueue<SocketWriteTask> &writelq = NetServer::getSocketWriteTaskQueue();

        string result;
        while(is_running)
        {
            QuerySearchTask task = lq.pop();
            //  	LogApp::getSelf().writeLog(LOG_LEVEL_INFO, "QuerySearch read: %s", key);
			string strQuery = task.data;
			//printf("%s\n", strQuery.c_str());
            string result = CInputTip::GetInstance().SearchTip(strQuery, m_iThreadId);
			printf("hit thread :%d\n", m_iThreadId);
            //string result = task.data;
            SocketWriteTask writeTask;
            writeTask.fd = task.fd;
            writeTask.len = result.size();
            writeTask.data = new char[result.size()+1];
            strcpy(writeTask.data,result.c_str());
            writeTask.data[result.size()] = '\0';
            delete[] task.data;
			writeTask.bHtml = task.bHtml;
            writelq.push(writeTask);	
        }	
    }
	void SetId(int id) {
		m_iThreadId = id;
	};
private:
	int m_iThreadId;
};

int CServerPlat::Run()
{
    Init();
	Process();
    return 0;
}

void CServerPlat::Init()
{
    m_iServerPort     = G_INT(serverport);
    m_iQueryThreadNum = G_INT(querythreadnum);
	string strCfgPath = G_STR(configurepath);
	string strPinyin  = strCfgPath + "pinyin";
	string strMPinyin = strCfgPath + "mpinyin";
	Pinyin::getInstance(strPinyin, strMPinyin);
}
int CServerPlat::Process()
{
    CThread thread;
    CThread dataupThread;
    CThread readThread;
    CThread queryThread[10];
    CThread writeThread;

    NetServer *server = new NetServer(m_iServerPort,1000,true);
    SocketNonBlockingReadThread *readServer = new SocketNonBlockingReadThread();
    TipDataUpdateThread *dataup = new TipDataUpdateThread();
    //SocketReadThread *readServer = new SocketReadThread();
    QuerySearchThread *query[m_iQueryThreadNum];
    SocketWriteThread *writer = new SocketWriteThread();

    if(thread.create((CRunnalble *)server)) {
        cout << "thread fail" <<endl;
        return -1;
    }
    if(dataupThread.create((CRunnalble *)dataup)) {
        cout << "thread fail" <<endl;
        return -1;
    }
    if(readThread.create((CRunnalble *)readServer)) {
        cout << "thread fail" <<endl;
        return -1;
    }
    for(int i=0; i<m_iQueryThreadNum; i++) {
        query[i] = new QuerySearchThread();
		query[i]->SetId(i);
        if(queryThread[i].create((CRunnalble *)query[i]))
        {
            cout << "thread fail" <<endl;
            return -1;
        }
    }
    if(writeThread.create((CRunnalble *)writer)) {
        cout << "thread fail" <<endl;
        return -1;
    }
    sleep(1000);
    thread.join();
    readThread.join();
    for(int i=0; i<m_iQueryThreadNum; i++) {
        queryThread[i].join();
    }
    writeThread.join();
    delete server;
    delete dataup;
    delete readServer;
    for(int i=0; i<m_iQueryThreadNum; i++) {
        delete query[i];
    }
    return 0;
}


string CServerPlat::ReadFromClient(int iFd, bool & bCgi)
{
        char mlse[9];
        bCgi = true;
        int ret = safe_read(iFd, mlse, 8);
        if (ret <4 ||*(int *)mlse!=0xFFFFEEEE) {
            //if(mlse[0] == 'G' && mlse[1] == 'E' && mlse[2] == 'T' && mlse[3] == ' ' && mlse[4] == '/' && mlse[5] == '?') {
            if(mlse[0] == 'G' && mlse[1] == 'E' && mlse[2] == 'T') {
                string strQuery = "";
                strQuery += mlse[6];
                strQuery += mlse[7];
                char c;
                while(1) { //read get
                    if(1 != read(iFd, &c, 1)) {
                        return "";
                    }
                    if(c == ' ') {
                        break;
                    }
                    strQuery += c;
                }
                string strTmp = "";
                while(1) { //read get
                    if(1 != read(iFd, &c, 1)) {
                        return "";
                    }
                    if(c == 0x0d || c == 0x0a) {
                        strTmp += c;
                        if(strTmp.size() == 4) break;
                        continue;
                    }
                    strTmp = "";
                }
                bCgi = false;
                return strQuery;
            }
            cout << "Read  fault package,the header is fault!!!" <<endl;
            return "";
        }
        int len;
        len = *(int *)(mlse+4);
        cout << "read len is:" << len <<endl;
        char* str = new char[len+1];
        str[len] = '\0';
        ret = safe_read(iFd, str, len);
        cout << "ret len is:" << ret << "\t" << len <<endl;
        if (ret < len) {
            //  log(LOG_SERVER, pack_log("str error:%d:%d", ret, len));
            delete []str;
            return "";
        }
	string strQuery = str;
        delete []str;
	return strQuery;
}
int CServerPlat::WriteToClient(int iFd, const string & strResult, bool bCgi)
{
    char data[1000000];
    int head = 0xFFFFEEEE;
    for(int i=0; i<4; i++)
        data[i] = ((char*)&head)[i];
    int size = strResult.size();

    if(!bCgi) {
        string strRet = "Content-type: text/plain;charset=gbk\n\n";
		strRet += "(" + strResult + ")";
		//strRet += "<?xml version='1.0' encoding='gbk' ?><responses code='200' msg='good'>" +  strResult + "</responses>";
        safe_write(iFd, strRet.c_str(), strRet.size());
    } else if(size < 1000000-8) {
        for(int i=0; i<4; i++)
            data[4+i] = ((char*)&size)[i];
        for(int i=0; i<size; i++)
            data[8+i] = strResult[i];
        safe_write(iFd, data, size+8);
    } else {
        size = 0;
        for(int i=0; i<4; i++)
        data[4+i] = ((char*)&size)[i];
        safe_write(iFd, data, 8);
    }
    return 0;
}

