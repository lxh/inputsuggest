#ifndef _COMPROGRAM_SERVER_NETSERVER_
#define _COMPROGRAM_SERVER_NETSERVER_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifdef USE_EPOLL 
#include <sys/epoll.h>
#endif
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <utility>

#include <iostream>
#include <string>
#include <set>

using namespace std;


class CRunnalble
{
public:

    CRunnalble():is_running(false)
    {
    }

    virtual ~CRunnalble()
    {
    }
public:
    virtual void run() = 0;
    virtual void stop()
    {
        is_running = false;
    }
    void set_running()
    {
        is_running = true;
    }
protected:
    bool is_running;
};

std::string getThreadId();

class CThread
{
public:
    CThread(): m_hThread(0), m_pid(0)
    {
    }
    ~CThread()
    {
    }		

protected:
    static void* _dispatch(void *arg)
    {
        CRunnalble* task = reinterpret_cast<CRunnalble*>(arg);
        task->set_running();
        task->run();

        pthread_exit((void**)0);
        return (void*)0;
    }

public:
    bool create(CRunnalble* task, bool bDetach = false) 
    {
        int nRet = pthread_create(&m_hThread, 0, _dispatch, task);
        if(bDetach)
            pthread_detach(m_hThread);
        return (nRet != 0);
    }			

    void join()
    {
        pthread_join(m_hThread, NULL);
    }

    pid_t getpid()
    {
        m_pid = ::getpid();
        return m_pid;
    }
public:
    static int			kill(pthread_t hThread){ return pthread_cancel(hThread); }

    static int			detach(pthread_t hThread){ return pthread_detach(hThread); }			

    static void			exit(){ pthread_exit(0); }

    static pthread_t	self(){return pthread_self();}
    static void sleep(int ms)
    {
        if (ms <= 1)
            return;
        struct timespec t1, t2;
        t1.tv_sec = (ms / 1000);
        t1.tv_nsec = (ms % 1000) * 1000000;
        int result = nanosleep(&t1, &t2);
        if (result < 0) {
            t1 = t2;
            nanosleep(&t1, &t2);
        }
    }
protected:
    pthread_t	m_hThread;	
    pid_t		m_pid;
};

class Mutex {

public:
    inline Mutex() {
        ::pthread_mutex_init(&mutex, NULL);
    }

    inline void lock() {
        ::pthread_mutex_lock(&mutex);
    }

    inline void unlock() {
        ::pthread_mutex_unlock(&mutex);
    }

    inline ~Mutex() {
        ::pthread_mutex_destroy(&mutex);
    }

private:
    pthread_mutex_t mutex;
    Mutex(const Mutex& m);
    Mutex& operator=(const Mutex &m);
};

/**
 *	definition of ScopedLock;
 **/
class ScopedLock {

public:
    inline ScopedLock(Mutex& mutex) :
        _mutex(mutex) {
            _mutex.lock();
        }

    inline ~ScopedLock() {
        _mutex.unlock();
    }

private:
    Mutex& _mutex;

};

template <typename T, size_t SIZE=1024>
class LockedQueue
{
    T queue[SIZE];
    size_t begin, end;
    pthread_mutex_t mutex;
    pthread_cond_t notempty, notfull;
public:
    LockedQueue():begin(0),end(0)
    {
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&notempty, NULL);
        pthread_cond_init(&notfull, NULL);
    }
    ~LockedQueue()
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&notempty);
        pthread_cond_destroy(&notfull);
    }
    void push(const T& t)
    {
        pthread_mutex_lock(&mutex);
        while (full())
        {
            pthread_cond_wait(&notfull, &mutex);
        }
        queue[end++] = t;
        if (end == SIZE)
        {
            end = 0;
        }
        pthread_cond_signal(&notempty);
        pthread_mutex_unlock(&mutex);
    }
    const T& pop()
    {
        pthread_mutex_lock(&mutex);
        while (empty())
        {
            pthread_cond_wait(&notempty, &mutex);
        }
        T &t = queue[begin++];
        if (begin == SIZE)
        {
            begin = 0;
        }
        pthread_cond_signal(&notfull);
        pthread_mutex_unlock(&mutex);
        return t;
    }
    bool empty()
    {
        return begin == end;
    }
    bool full()
    {
        return (end+1+SIZE)%SIZE == begin;
    }
};

class Lock
{
    pthread_mutex_t mutex;
public:
    Lock()
    {
        pthread_mutex_init(&mutex, NULL);
    }
    ~Lock()
    {
        pthread_mutex_destroy(&mutex);
    }
    void lock()
    {
        pthread_mutex_lock(&mutex);
    }
    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }
};		

class NetServer;

struct SocketReadTask
{
    NetServer *server;
    int fd;
};
struct SocketWriteTask
{
    int fd;
	bool bHtml;
    char *data;
    size_t len;
};

struct QuerySearchTask
{
    int fd;	
	bool bHtml; //1 来源于html
    char* data;
    size_t len;
};

//priorityQueue, indexReader
struct QueryResultTask
{

};


#ifndef USE_SELECT
class FDEvent
{
    int maxfd;
    bool nonblocking;
    int epet;
    fd_set read_set;
    fd_set tset;
    Lock l;
    int sit;
public:
    typedef int EventType;
    FDEvent(int max_client_num, bool is_nonblocking):maxfd(max_client_num),nonblocking(is_nonblocking)
    {
        FD_ZERO(&read_set);
    }
    ~FDEvent()
    {
    }
    int wait(int timeout)
    {
        struct timeval* tv = NULL, tt;
        if (timeout != -1)
        {
            tt.tv_sec = timeout/1000;
            tt.tv_usec = (timeout%1000)/1000;
            tv = &tt;
        }
        else
        {
            tt.tv_sec = 0;
            tt.tv_usec = 1000;
            tv = &tt;
        }
        l.lock();
        tset = read_set;
        l.unlock();
        int n = select(maxfd+1, &tset, NULL, NULL, tv);
        return n;
    }
    int add(int fd, bool init = false)
    {
        l.lock();
        if (fd>maxfd) maxfd = fd;
        FD_SET(fd, &read_set);
        l.unlock();
        return 0;
    }
    int del(int fd)
    {
        l.lock();
        FD_CLR(fd, &read_set);
        l.unlock();
        return 0;
    }
    pair<int, EventType> get(size_t i)
    {
        l.lock();
        if (i == 0)
        {
            sit = 0;
        }
        for (;sit < maxfd+1;++sit)
        {
            if (FD_ISSET(sit, &tset))
            {
                int p = sit;
                ++sit;
                l.unlock();
                return make_pair(p, 1);
            }
        }
        l.unlock();
        return make_pair(-1, 0);
    }
    bool isEventIn(int t)
    {
        return t&1;
    }
};
#else
class FDEvent
{
    int evfd;
    struct epoll_event *events;
    int maxfds;
    bool nonblocking;
    int epet;
public:
    typedef int EventType;
    FDEvent(int max_client_num, bool is_nonblocking):maxfds(max_client_num),nonblocking(is_nonblocking)
    {
        if ((evfd = epoll_create(maxfds)) == -1)
        {
            perror("epoll_create error!");
            exit(1);
        }
        if (fcntl(evfd, F_SETFD, FD_CLOEXEC) == -1)
        {
            perror("epoll_create error!");
            exit(1);
        }
        events = new epoll_event[max_client_num];
        epet = nonblocking?EPOLLET:0;
    }
    ~FDEvent()
    {
        delete[] events;
    }
    int wait(int timeout)
    {
        return epoll_wait(evfd, events, maxfds, timeout);
    }
    int add(int fd, bool init = false)
    {
        epoll_event ev;
        if (init)
        {
            ev.events = EPOLLIN;
        }
        else
        {
            ev.events = EPOLLIN|epet;
        }
        ev.data.u64 = 0;
        ev.data.fd = fd;
        if (epoll_ctl(evfd, EPOLL_CTL_ADD, fd, &ev) == -1)
        {
            perror("epoll_ctl error!");
            return -1;
        }
        return 0;
    }
    int del(int fd)
    {
        epoll_event ev;
        ev.data.fd = fd;
        if (epoll_ctl(evfd, EPOLL_CTL_DEL, fd, &ev) == -1)
        {
            perror("epoll_ctl error!");
            return -1;
        }
        return 0;
    }

    pair<int, EventType> get(size_t i)
    {
        return make_pair(events[i].data.fd, events[i].events);
    }
    bool isEventIn(int t)
    {
        return t&EPOLLIN;
    }
};
#endif

class NetServer : public CRunnalble
{
    int listenfd, kdpfd;
    struct sockaddr_in my_addr;
    struct sockaddr_in remote_addr;
    FDEvent fde;
    int port;
    bool nonblocking;
    size_t clientno;
public:
    NetServer(int port, int max_client_num, bool is_nonblocking = false):fde(max_client_num, is_nonblocking), clientno(0)
    {   
        this->port = port;
        listenfd = socket(AF_INET, SOCK_STREAM, 0); 
        assert(listenfd != -1);
        my_addr.sin_family=AF_INET;
        my_addr.sin_port=htons(port);
        my_addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(my_addr.sin_zero),8);
        int flag = 1;
        if (is_nonblocking)
        {   
            nonblocking = true;
        }   
        else
        {   
            nonblocking = false;
        }   
        if( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1) 
        {   
            perror("setsockopt");
            exit(1);
        }   



        struct linger    optval;
        optval.l_onoff = 1;
        optval.l_linger = 60; 

        if( setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &optval, sizeof(struct linger)) == -1) 
        {   
            perror("setsockopt linger");
            exit(1);
        }
        if (bind(listenfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
        {
            perror("bind");
            exit(1);
        }
        if (listen(listenfd, max_client_num) == -1)
        {
            perror("listen");
            exit(1);
        }
        if (nonblocking && fcntl(listenfd, F_SETFL, fcntl(listenfd, F_GETFD, 0)|O_NONBLOCK) == -1)
        {
            perror("fcntl");
            exit(1);
        }
        int ret = fde.add(listenfd, true);

        assert(ret != -1);
        cout <<  "server started at port:"<<"\t"<< port<<endl;
    }
    ~NetServer()
    {
    }
    virtual void run()
    {
        LockedQueue<SocketReadTask>& tq = getSocketReadTaskQueue();
        while (is_running)
        {
            int n = fde.wait(-1);
            socklen_t adsize = sizeof(remote_addr);
            for (int i=0; i<n; i++)
            {
                pair<int, FDEvent::EventType> p = fde.get(i);
                if (p.first == listenfd)
                {
                    clientno++;
                    int clientfd = accept(listenfd, (struct sockaddr *)&remote_addr, &adsize);


                    if (clientfd == -1)
                    {
                        perror("accept error!");
                        continue;
                    }
                    if (nonblocking && fcntl(clientfd, F_SETFL, fcntl(clientfd, F_GETFD, 0)|O_NONBLOCK))
                    {
                        perror("set noblocking error!");
                        close(clientfd);
                        continue;
                    }
                    fde.add(clientfd);
                }
                else if (fde.isEventIn(p.second))
                {   
                    fde.del(p.first);
                    SocketReadTask task;
                    task.server = this;
                    task.fd = p.first;
                    tq.push(task);
                }
                else
                {
                }
            }
        }
    }
    int addSocket(int fd)
    {
        return fde.add(fd);
    }
    int getPort()
    {
        return port;
    }
    static LockedQueue<SocketReadTask>& getSocketReadTaskQueue()
    {
        static LockedQueue<SocketReadTask> socket_read_task_queue;
        return socket_read_task_queue;
    }
    static LockedQueue<SocketWriteTask>& getSocketWriteTaskQueue()
    {
        static LockedQueue<SocketWriteTask> socket_write_task_queue;
        return socket_write_task_queue;
    }
    static LockedQueue<QuerySearchTask>& getQuerySearchTaskQueue()
    {
        static LockedQueue<QuerySearchTask> query_search_task_queue;
        return query_search_task_queue;
    }
    static LockedQueue<QueryResultTask>& getQueryResultTaskQueue()
    {
        static LockedQueue<QueryResultTask> query_result_task_queue;
        return query_result_task_queue;
    }
};

inline ssize_t safe_read(int fd, void* buf, size_t size) {
    ssize_t cc;
    size_t sz = size;
    char* bp =(char *) buf;
    do {
        timeval startt,finisht;
        gettimeofday(&startt,0);
        long starttime = (startt.tv_sec*1000 + startt.tv_usec/1000);
        cc = read(fd, bp, sz);
        cout << "safe read size is!!!:" << cc <<endl;
        if ( cc > 0 ) {
            bp += cc;
            sz -= cc;
        } else if ( cc==0 )	// EOF
            return (sz) ? (size-sz) : 0;
        else if ( cc < 0 && errno != EINTR && errno != EAGAIN)
            return cc;
        gettimeofday(&startt,0);
        long endtime = (startt.tv_sec*1000 + startt.tv_usec/1000);
        cout << "safe read time is" <<  endtime-starttime<<endl;
    } while (sz > 0);
    return size;
}

inline ssize_t safe_write(int fd, const void* buf, size_t size) {
    ssize_t cc;
    size_t sz = size;
    const char *bp = (const char *)buf;
    if ( size==0 )	return 0;
    do {
        timeval startt,finisht;
        gettimeofday(&startt,0);
        long starttime = (startt.tv_sec*1000 + startt.tv_usec/1000);
        cc = write(fd, bp, sz);
        cout << "safe write size is!!!:" << cc <<endl;
        if ( cc > 0 ) {
            bp += cc;
            sz -= cc;
        } else if ( cc < 0 && errno != EINTR && errno != EAGAIN)
            return cc;
        gettimeofday(&startt,0);
        long endtime = (startt.tv_sec*1000 + startt.tv_usec/1000);
        cout << "safe write time is" <<  endtime-starttime<<endl;
    } while (sz > 0);
    return size;
}

inline ssize_t read_block(int fd, void* buf, size_t size)
{
    int num = size/15000;
    int lost = size - num*15000;
    char* bp =(char *) buf;
    for(int i=0; i<num; i++)
    {
        safe_read(fd,bp,15000);
        bp += 15000;
    }
    safe_read(fd,bp,lost);
    bp += lost;
    return 0;
}

inline ssize_t write_block(int fd, void* buf, size_t size)
{
    int num = size/15000;
    int lost = size - num*15000;
    const char *bp = (const char *)buf;
    for(int i=0; i<num; i++)
    {
        safe_write(fd,bp,15000);
        bp += 15000;
    }
    safe_write(fd,bp,lost);
    return 0;
}


class SocketReadThread : public CRunnalble 
{
protected:
    //  MessageHandler& handler;
public:
    //  SocketReadThread(MessageHandler& h):handler(h)
    SocketReadThread()
    {
    }
    virtual void run()
    {
        LockedQueue<SocketReadTask> &lq = NetServer::getSocketReadTaskQueue();
        LockedQueue<QuerySearchTask> &querylq = NetServer::getQuerySearchTaskQueue();
        while (is_running)
        {
            SocketReadTask task = lq.pop();
            char mlse[9];
            mlse[9] = '\0';
            /* 
               int nZero=0;
               int nRecvBuf=1000*1024;
               if(setsockopt(task.fd,SOL_SOCKET,SO_RCVBUF,(const char*)nRecvBuf,sizeof(int)) == -1)
               {
               cout << "set error!" << Errno <<endl;
               }*/

            int ret = safe_read(task.fd, mlse, 8);
            if (ret <4 ||*(int *)mlse!=0xFFFFEEEE)
            {
				ReadFromHtml(querylq, task.fd, mlse);
				continue;
            }
            int len;
            len = *(int *)(mlse+4);
            /*ret = safe_read(task.fd, &len, 4);
              if (ret < 4)
              {
            //  log(LOG_SERVER, "len error");
            close(task.fd);
            continue;
            }*/
            cout << "read len is:" << len <<endl;
            char* str = new char[len+1];
            str[len] = '\0';
            ret = safe_read(task.fd, str, len);
            cout << "ret len is:" << ret << "\t" << len <<endl;
            if (ret < len)
            {
                //  log(LOG_SERVER, pack_log("str error:%d:%d", ret, len));
                close(task.fd);
                continue;
            }
            QuerySearchTask queryTask;
			queryTask.bHtml = false;
            queryTask.fd = task.fd;
            queryTask.data = str;
            queryTask.len = len;
            querylq.push(queryTask);	
            //  handler.handleMessage(task.server, task.fd, str, len);
            //      process request
        }
    };
public:
	int ReadFromHtml(LockedQueue<QuerySearchTask> &querylq, int &iFd, char mlse[9], int iFlag=0) {
		if(mlse[0] != 'G' || mlse[1] != 'E' || mlse[2] != 'T') {
			printf("----->>>%c%c%c\n", mlse[0],mlse[1],mlse[2]);
			cout << "Read  fault package,the header is fault!!!" <<endl;
			close(iFd);
			return -1;
		}
		string strQuery = "";
		strQuery += mlse[6];
		strQuery += mlse[7];
		if(iFlag == 0) {
			char c;
			while(1) { //read get
				if(1 != read(iFd, &c, 1)) {
					cout << "read get error" << endl;
					close(iFd);
					return -1;
				}
				printf("-%c\n", c);
				if(c == ' ') {
					break;
				}
				strQuery += c;
			}
			string strTmp = "";
			while(1) { //read other
				if(1 != read(iFd, &c, 1)) {
					cout << "read other error" << endl;
					close(iFd);
					return -1;
				}
				if(c == 0x0d || c == 0x0a) {
					strTmp += c;
					if(strTmp.size() == 4) break;
					continue;
				}
				strTmp = "";
			}
		} else { //已经全部读取出来了
			int iLoop;
			strQuery = "";
			printf("=====%s\n", mlse);
			iLoop = 4;
			for(; iLoop < iFlag; iLoop++) {
				if(mlse[iLoop] == ' ') {
					break;
				}
				strQuery += mlse[iLoop];
			}
			if(iLoop >= iFlag) {
				cout << "Read  html head error!!!" <<endl;
				close(iFd);
				return -1;
			}
			printf("----->>query:%s\n", strQuery.c_str());
		}
		int iLoop;
		//GET http://x.x.x.x:31415/?no=1&x=xuan&p=11111
		for(iLoop = 0; iLoop < strQuery.size(); iLoop++) { //查找?
			if(strQuery[iLoop] == '?') {
				break;
			}
		}
		if(iLoop > strQuery.size() - 1) {
			cout << "not found ? error" << endl;
			close(iFd);
			return -1;
		}
		iLoop++;
		string strNew = ConvertString(strQuery.substr(iLoop).c_str());
		char *szStr = new char[strNew.size() + 1];
		for(iLoop = 0; iLoop < strNew.size(); iLoop++) {
			szStr[iLoop] = strNew[iLoop];
		}
		szStr[iLoop] = 0;
		QuerySearchTask queryTask;
		queryTask.fd = iFd;
		queryTask.bHtml = true;
		queryTask.data = szStr;
		queryTask.len = strNew.size();
		querylq.push(queryTask);	
		return 0;
	};
	string ConvertString(const char *str) {
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
	};
	int htoi(char *s) {
		const char *digits = "0123456789ABCDEF";

		if (islower(s[0])) s[0]=toupper(s[0]);
		if (islower(s[1])) s[1]=toupper(s[1]);

		return 16 * (strchr(digits, s[0])-strchr(digits,'0')) +(strchr(digits,s[1])-strchr(digits,'0'));
	};

};

class SocketNonBlockingReadThread:public SocketReadThread
{
    FDEvent fde;
    const static size_t BUF_SIZE = 655360;
public:
    // SocketNonBlockingReadThread(MessageHandler& h):SocketReadThread(h),fde(1, true)
    SocketNonBlockingReadThread():SocketReadThread(),fde(1, true)
    {
    }
    virtual void run()
    {
        cout << "Read server started !!!" <<endl;
        LockedQueue<SocketReadTask> &lq = NetServer::getSocketReadTaskQueue();
        LockedQueue<QuerySearchTask> &querylq = NetServer::getQuerySearchTaskQueue();
        while (is_running)
        {
            SocketReadTask task = lq.pop();
            timeval startt,finisht;
            gettimeofday(&startt,0);
            long starttime = (startt.tv_sec*1000 + startt.tv_usec/1000);
            char buf[BUF_SIZE], *p = buf;
            int pos = 0, remain;
            int ret;
            size_t size = 0;
            bool is_longdata = false;
            char *longdata = NULL;
            size_t long_pos = 0;
loop:
            do
            {
                remain = BUF_SIZE-pos;
                ret = read(task.fd, buf+pos, remain);
                cout << "read size is:!!!" << ret <<endl;
                if (ret == -1) 
                {
                    cout << "Read socket fault !!!" <<endl;
                    //	close(task.fd);
                    break;
                }
                p = buf;
                //data is loger than 65535
                if (ret > 0 && is_longdata)
                {
                    if (size > ret+pos+long_pos)
                    {
                        memcpy(longdata+long_pos, buf, ret+pos);
                        long_pos+=ret+pos;
                        pos = 0;
                        continue;
                    }
                    else
                    {
                        memcpy(longdata+long_pos, buf, size-long_pos);
                        p = buf+size-long_pos;
                        long_pos+=size-long_pos;
                        //  handler.handleMessage(task.server, task.fd, longdata, size);
                        //      process request
                        is_longdata = false;
                    }
                }
                while (p+8 <= buf+ret+pos)
                {
                    if (*(int *)p != 0xFFFFEEEE)
                    {
						if(BUF_SIZE == remain) {
							//cout << "Read  fault package,the header is fault!!!" <<endl;
							int iRet = ReadFromHtml(querylq, task.fd, (char*)p, ret);
							if(iRet == 0) {
								//task.server->addSocket(task.fd);
							}
							goto overloop;
						}
                        ret = 0;
                        break;
                    }
                    size = *(int *)(p+4);

                    if (size > BUF_SIZE)
                    {
                        is_longdata = true;
                        longdata = new char[size+1];
                        longdata[size] = '\0';
                        long_pos = buf+ret+pos-p-8;
                        memcpy(longdata, p+8, long_pos);
                        p += long_pos+8;
                    }
                    //size is length, now temp num is 80	
                    else if (p+size+8<=buf+ret+pos)
                    {          
                        char *str = new char[size+1];
                        str[size] = '\0';
                        memcpy(str, p+8, size);
                        //  handler.handleMessage(task.server, task.fd, str, size);
                        //      process request
                        QuerySearchTask queryTask;
                        queryTask.fd = task.fd;
                        queryTask.data = str;
                        queryTask.len = size;
                        querylq.push(queryTask);	
                        p +=size+8;
                    }
                    else
                    {
                        //add log yuanzy
                        break;
                    }
                }
                pos = buf+ret+pos-p;
                if (p != buf && pos > 0)
                {
                    memmove(buf, p, pos);
                }
            }while(ret == remain);

#if 1 
            if (ret == -1)
            {
                if (errno == EAGAIN) {
                    if (is_longdata || (pos != 0)) {
                        fde.add(task.fd);
waitloop1:
                        int n = fde.wait(-1);
                        if (n == -1) {
                            cout << "wait error!"<<endl;
                            goto waitloop1;
                        } else if (n == 0) {
                            goto waitloop1;
                        }
                        fde.del(task.fd);
                        goto loop;
                    } else {
                        task.server->addSocket(task.fd);
                    }
                } else {
                    if (is_longdata) {
                        delete[] longdata;
                    }
                    cout << "socket error" <<endl;
                    close(task.fd);
                }
            } else if (ret == 0) {
                if (is_longdata)
                {
                    delete[] longdata;
                }
                close(task.fd);
            } else if (pos == 0 && !is_longdata) {
                task.server->addSocket(task.fd);
            } else {
                fde.add(task.fd);
waitloop2:
                int n = fde.wait(-1);
                if (n == -1)
                {
                    cout << "wait error!" <<endl;
                    goto waitloop2;
                }
                else if (n == 0)
                {
                    goto waitloop2;
                }
                fde.del(task.fd);
                goto loop;
            }
#endif
overloop:
            gettimeofday(&startt,0);
            long endtime = (startt.tv_sec*1000 + startt.tv_usec/1000);
            cout << "read time is" <<  endtime-starttime<<endl;

        }
    }
};

class SocketWriteThread : public CRunnalble 
{
private:
	void WriteHtml(SocketWriteTask & task) {
		static string strHead = "jQueryBack&&jQueryBack(\n";
		static string strTail = ")\n";
		int iLength = strHead.size() + strTail.size() + task.len;
		char szLength[10];
		sprintf(szLength, "%d", iLength);
		string s = "HTTP/1.1 200 OK\r\n";
		s += "Server: R++ Web Server\r\n";
		s += "Content-Length: ";
		s += szLength;
		s += "\r\n"; 
		s += "Content-type: text/plain;charset=gbk\r\n\r\n";
		safe_write(task.fd, s.c_str(), s.size());
		safe_write(task.fd, strHead.c_str(), strHead.size());
		safe_write(task.fd, task.data, task.len);
		safe_write(task.fd, strTail.c_str(), strTail.size());
		close(task.fd);
	};
public:
    virtual void run()
    {
        cout << "Writer server started !!!" <<endl;
        LockedQueue<SocketWriteTask> &lq = NetServer::getSocketWriteTaskQueue();
        int head = 0xFFFFEEEE;
        int size = 0;
        while (is_running)
        {
            SocketWriteTask task = lq.pop();
			if(task.bHtml) {
				WriteHtml(task);
				continue;
			}

            timeval startt,finisht;
            gettimeofday(&startt,0);
            long starttime = (startt.tv_sec*1000 + startt.tv_usec/1000);
            char data[1000000];
            for(int i=0; i<4; i++)
                data[i] = ((char*)&head)[i];
            size = task.len;

            if(size < 1000000-8)
            {
                for(int i=0; i<4; i++)
                    data[4+i] = ((char*)&size)[i];
                for(int i=0; i<size; i++)
                    data[8+i] = task.data[i];
                safe_write(task.fd, data, task.len+8);
            }
            else
            {
                size = 0;
                for(int i=0; i<4; i++)
                    data[4+i] = ((char*)&size)[i];
                safe_write(task.fd, data, 8);

            }
            gettimeofday(&startt,0);
            long endtime = (startt.tv_sec*1000 + startt.tv_usec/1000);
            cout << "write time is" <<  endtime-starttime<<endl;

            //长度不相等，增加log,closesocket  yuanzy

            // safe_write(task.fd, &task.len, 4);
            // safe_write(task.fd, task.data, task.len);
			//这里增加close出错
			//close(task.fd);
            delete[] task.data;
        }
    }
};

#endif                                        
