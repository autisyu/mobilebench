#include"worker.h"
Worker::Worker(int request, const char* serv_ip, int serv_port)
{
    LOG(stderr, "serv_ip = %s, serv_port = %d, request = %d\n", serv_ip, serv_port, request);
    num_connection = request;
    config_server(serv_ip, serv_port);
    events         = new epoll_event[num_connection];
    res_assert(events, "new epoll_event\n");

    connptr        = new connection[num_connection];
    res_assert(connptr, "new connection\n");

    epfd           = epoll_create(num_connection);
    sys_assert(epfd, "epoll_create");
}
void Worker::config_server(const char *serv_ip, int serv_port)
{
   char server_ip[64];
   memmove(server_ip, serv_ip, strlen(serv_ip));
   LOG(stderr, "config_server %s\n", server_ip);
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family           = AF_INET;
   server_addr.sin_addr.s_addr      = inet_addr(server_ip);
   server_addr.sin_port             = htons(serv_port);
}

Worker::~Worker()
{
    delete[] events;
    delete[] connptr;
    events  = NULL;
    connptr = NULL;
}

void Worker::start()
{
    do_work();
}
void Worker::dispatch()
{
    long time_out = -1;
    time_out      = get_timeout();
    int res, i, rstatus;
    while (1) {
        LOG(stderr, "start event loop\n");
        res             = epoll_wait(epfd, events, num_connection, time_out);
	sys_assert(res, "dispatch, epoll_wait");
	LOG(stderr, "return event = %d\n", res);

	for (i = 0; i < res; ++i) {
	    connection *c   = (connection*)events[i].data.ptr; 
	    LOG(stderr, "return fd= %d, call_back = %p\n", c->fd, c->call_back);
	    rstatus         = c->call_back(c->fd, events[i].events, c->arg);
	    switch (rstatus) {
	        case STATESWITCH:
		    LOG(stderr, "STATESWITCH\n");
	            connection_mod(c);
		    break;
		case STATEERROR:
		    reconnect(c->conn_id);
		    break;
	    }
	}
        LOG(stderr, "end event loop\n");
	timeout_process();
    }
}
void Worker::do_work()
{
    int conn_count = 0;
    while (conn_count < num_connection) {
       connection_add(conn_count,EPOLLOUT); 
       conn_count++;
    }
    dispatch();
}

void Worker::connection_add(int conn_num, int events)
{
   LOG(stderr, "add conn_num = %d\n", conn_num);
   connection_set(conn_num, events);
   struct epoll_event epv = {0, {0}};
   epv.data.ptr           = &connptr[conn_num];
   epv.events             = connptr[conn_num].events ;
   int res                = epoll_ctl(epfd, EPOLL_CTL_ADD, connptr[conn_num].fd, &epv);
   sys_assert(res, "connection_add, epoll_ctl");
}

void Worker::connection_set(int conn_num, int events)
{
   struct timeval now;
   gettimeofday(&now, NULL);
   int client_fd              = socket(AF_INET, SOCK_STREAM, 0);
   setsocketopt()
   sys_assert(client_fd, "connection_set, socket");
   int timeout                = rand() % 4;
   strcpy(connptr[conn_num].packet, "hello world");
   connptr[conn_num].conn_id  = conn_num;
   connptr[conn_num].time     = timeout;
   now.tv_sec                += timeout;
   connptr[conn_num].timeout  = now; 
   connptr[conn_num].events   = events;
   connptr[conn_num].state    = STATEW;
   connptr[conn_num].call_back= CallbackA::state_process;
   connptr[conn_num].arg      = &connptr[conn_num];
   int res                    = connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
   LOG(stderr, "connect res = %d, client_fd = %d\n", res, client_fd);
   sys_assert(res, "connection_set, connect");
   connptr[conn_num].fd       = client_fd; 

}

void Worker::connection_mod(connection* c)
{
    LOG(stderr, "connection_mod, c = %p\n", c);
    struct epoll_event epv = {0, {0}};
    epv.events             = c->events;
    epv.data.ptr           = c;
    int res                = epoll_ctl(epfd, EPOLL_CTL_MOD, c->fd, &epv);
    sys_assert(res, "connection_mod, epoll_ctl");
}

void Worker::connection_del(int conn_num)
{
    connection conn_ptr    = connptr[conn_num];
    struct epoll_event epv = {0, {0}};
    int res                = epoll_ctl(epfd, EPOLL_CTL_DEL, conn_ptr.fd, &epv);
    sys_assert(res, "connection_del, epoll_ctl");
    close(conn_ptr.fd);
}
void Worker::reconnect(int conn_num)
{
    int events   = EPOLLOUT;
    connection_del(conn_num);
    connection_add(conn_num, events);
}
void Worker::timeout_process()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    int conn_count = 0;
    while (conn_count < num_connection) {
        if (connptr[conn_count].timeout.tv_sec < now.tv_sec && connptr[conn_count].time) {
	    reconnect(conn_count);
	    LOG(stderr, "timeout");
	    getchar();
	    getchar();
	}
	conn_count++;
    }
}
int Worker::get_timeout()
{
    return 2;
}
void Worker::signal_process()
{

}
