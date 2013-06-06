#ifndef _WORKER_H
#define _WORKER_H
#include<algorithm>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<memory.h>
#include<stdio.h>
#include<stdlib.h>
//#include<file.h>
#include<vector>
#include<functional>
#include<queue>
#include<sys/time.h>
#include<malloc.h>
#include<sys/epoll.h>
#include<arpa/inet.h>
#include<stdarg.h>
#include "util.h"
#include "connection.h"
#include "call-back.h"
class WorkerInfo
{
    private:
        int m_rpipe;
        int m_wpipe;
        int m_num_connection;
	int m_max_connection;
	int m_id;
    public:
        void inline set_rpipe(int fd) {m_rpipe = fd;}
        int  inline get_rpipe() {return m_rpipe;}
        void inline set_id(int id) {m_id = id;}
        int  inline get_id() {return m_id;}
        void inline set_wpipe(int fd) {m_wpipe = fd;}
        int  inline get_wpipe() {return m_wpipe;}
        void inline set_num_connection (int conn_num) {m_num_connection = conn_num;}
	int  inline get_num_connection() {return m_num_connection;}  
        void inline set_max_connection (int conn_num) {m_max_connection = conn_num;}
        int  inline get_max_connection () {return m_max_connection;}
};
class Worker
{
    private:
        struct epoll_event  *m_events;
        std::priority_queue<int,std::vector<int>, std::greater<int> >     m_heap_timer;
        struct connection   *m_connptr, m_monitor;
	int                  m_epfd;
        char                 m_server_ip[64];
	int                  m_server_port;
	struct sockaddr_in   m_server_addr;
	int                  m_logfd;
    public:
        WorkerInfo           m_info;
    public:
        Worker(const char *server_ip, int server_port);
        ~Worker();
	void Start();
    private:
        void WriteLog(const char *fmt, ...);
	void StartLog();
        void ConfigServer(const char *serv_ip, int serv_port);
        void Reconnect(int conn_num);
        void DoWork();
        void ConnectionAdd(int conn_num, int events, int type);
        void ConnectionDel(int conn_num);
        void ConnectionSet(int conn_num, int events, int type);
        void ConnectionMod(connection *c);
        void TimeoutProcess();
	int  GetTimeout();
	int  AddTimer(int timeout, int conn_num);
	int  AddConnection(int timeout, int conn_num);
        int  CommandProcess(int fd, int events, void* arg);
	void AddMonitor();
	void Dispatch();
};
#endif /*_WORKER_H*/
