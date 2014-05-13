#ifndef _SERVER_PLAT_H_
#define _SERVER_PLAT_H_
#include <stdio.h>

class CServerPlat {
public:
    int Run();
private:
    void Init();
    int Process();

    string ReadFromClient(int iFd, bool & bCgi);
    int WriteToClient(int iFd, const string & strResult, bool bCgi);
private:
    int m_iServerPort;     //服务器的端口号
    int m_iQueryThreadNum; //查询线程的个数
};


#endif
