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
        std::vector<int>     m_heap_timer;
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
	void start();
    private:
        void write_log(const char *fmt, ...);
	void start_log();
        void config_server(const char *serv_ip, int serv_port);
        void reconnect(int conn_num);
        void do_work();
        void connection_add(int conn_num, int events, int type);
        void connection_del(int conn_num);
        void connection_set(int conn_num, int events, int type);
        void connection_mod(connection *c);
        void timeout_process();
	int  get_timeout();
	int  add_timer(int timeout, int conn_num);
	int  add_connection(int timeout, int conn_num);
        int  command_process(int fd, int events, void* arg);
	void add_monitor();
	void dispatch();
};
#endif /*_WORKER_H*/
