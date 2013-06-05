#ifndef _WORKER_H
#define _WORKER_H
#include<algorithm>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<memory.h>
#include<stdio.h>
#include<stdlib.h>
#include<vector>
#include<sys/time.h>
#include<malloc.h>
#include<sys/epoll.h>
#include<arpa/inet.h>
#include "util.h"
#include "connection.h"
#include "call-back.h"
class Worker
{
    private:
        struct epoll_event *events;
        int num_connection;
        std::vector<int>  heap_timer;
        struct connection *connptr;
        int pipe;
	int epfd;
    public:
        char server_ip[64];
	int  server_port;
	struct sockaddr_in server_addr;
    public:
        Worker(int request, const char *server_ip, int server_port);
        ~Worker();
	void start();

    private:
        void config_server(const char *serv_ip, int serv_port);
        void reconnect(int conn_num);
        void do_work();
        void connection_add(int conn_num, int events);
        void connection_del(int conn_num);
        void connection_set(int conn_num, int events);
        void connection_mod(connection *c);
        void timeout_process();
	int  get_timeout();
        void signal_process();
	void dispatch();
};
#endif /*_WORKER_H*/
