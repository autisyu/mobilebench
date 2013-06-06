#include"worker.h"
Worker::Worker(const char* serv_ip, int serv_port)
{
    int max_connection = 70000;
    int num_connection = 0;
    m_info.set_num_connection(num_connection);
    m_info.set_max_connection(max_connection);
    write_log("%d\n", m_info.get_max_connection());
    config_server(serv_ip, serv_port);
    m_events         = new epoll_event[max_connection];
    res_assert(m_events, "new epoll_event\n");

    m_connptr        = new connection[max_connection];
    res_assert(m_connptr, "new connection\n");

    m_epfd           = epoll_create(num_connection + 1);
    sys_assert(m_epfd, "epoll_create");
}

void Worker::start_log()
{
    char log_file_name[100];
    sprintf(log_file_name, "%d_worker_log", m_info.get_id());
    printf("log_file_name = %s, id = %d\n", log_file_name, m_info.get_id());
    m_logfd            = open(log_file_name, O_CREAT | O_WRONLY, 0644);
    if (-1 == m_logfd) {
        perror("fopen");
	exit(-1);
    } else {
        dup2(m_logfd, STDERR_FILENO);
    }

}
void Worker::config_server(const char *serv_ip, int serv_port)
{
   write_log("serv_ip = %s, serv_port = %d\n", serv_ip, serv_port);
   m_server_port = serv_port;
   memmove(m_server_ip, serv_ip, strlen(serv_ip));
   write_log("config_server %s\n", m_server_ip);

   memset(&m_server_addr, 0, sizeof(m_server_addr));
   m_server_addr.sin_family           = AF_INET;
   m_server_addr.sin_addr.s_addr      = inet_addr(m_server_ip);
   m_server_addr.sin_port             = htons(m_server_port);
}
Worker::~Worker()
{
    delete[] m_events;
    delete[] m_connptr;
    m_events  = NULL;
    m_connptr = NULL;
    close(m_logfd);
}
void Worker::add_monitor()
{
    m_monitor.fd        = m_info.get_rpipe();
    //m_monitor.call_back = Worker::command_process;
    epoll_event epv     = {0, {0}};
    epv.events          = EPOLLIN;
    epv.data.ptr        = &m_monitor;
    epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_monitor.fd, &epv);
}
void Worker::start()
{
    add_monitor();
    start_log();
    do_work();
}

void Worker::do_work()
{
    dispatch();
}

void Worker::dispatch()
{
    long time_out = -1;
    time_out      = get_timeout();
    int res, i, rstatus;
    while (1) {
        write_log("start event loop, m_epfd = %d, m_events = %p, max_conn = %d\n", m_epfd, m_events, m_info.get_max_connection());
        res             = epoll_wait(m_epfd, m_events, m_info.get_max_connection(), time_out);
	sys_assert(res, "dispatch, epoll_wait");
	write_log("return event = %d\n", res);

	for (i = 0; i < res; ++i) {
	    connection *c   = (connection*)m_events[i].data.ptr; 
	    if (c == &m_monitor) {
	        command_process(c->fd, m_events[i].events, c->arg);
		continue;
	    }
	    write_log("return fd= %d, call_back = %p\n", c->fd, c->call_back);
	    rstatus         = c->call_back(c->fd,m_events[i].events, c->arg);
	    switch (rstatus) {
	        case S_SWITCH:
		    write_log("STATESWITCH\n");
	            connection_mod(c);
		    break;
		case S_ERROR:
		    reconnect(c->conn_id);
		    break;
	    }
	}
        write_log("end event loop\n");
	timeout_process();
    }
}

void Worker::connection_add(int conn_num, int events, int type)
{
   write_log("add conn_num = %d\n", conn_num);
   struct connection *connptr = m_connptr;
   connection_set(conn_num, events, type);
   struct epoll_event epv = {0, {0}};
   epv.data.ptr           = &connptr[conn_num];
   epv.events             = connptr[conn_num].events ;
   int res                = epoll_ctl(m_epfd, EPOLL_CTL_ADD, connptr[conn_num].fd, &epv);
   sys_assert(res, "connection_add, epoll_ctl");
}

void Worker::connection_set(int conn_num, int events, int type)
{
   struct timeval now;
   struct connection *connptr = m_connptr; 
   gettimeofday(&now, NULL);
   int client_fd              = socket(AF_INET, SOCK_STREAM, 0);
   //fcntl(client_fd, F_SETFL, O_NONBLOCK);
   sys_assert(client_fd, "connection_set, socket");
   int timeout                = rand() % 4;
   strcpy(connptr[conn_num].packet, "hello world");
   connptr[conn_num].conn_id  = conn_num;
   //connptr[conn_num].time     = timeout;
   now.tv_sec                += timeout;
   connptr[conn_num].timeout  = now; 
   connptr[conn_num].events   = events;
   connptr[conn_num].state    = S_W;
   connptr[conn_num].call_back= CallbackA::state_process;
   connptr[conn_num].type     = type;
   connptr[conn_num].arg      = &connptr[conn_num];
   
   int res                    = connect(client_fd, (struct sockaddr*)&m_server_addr, sizeof(m_server_addr));
   /*int retry                  = 0;
   while (-1 ==(res= connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)))) {
       retry++;
   }*/

   //write_log(m_logfd, "connect res = %d, client_fd = %d, retry = %d\n", res, client_fd, retry);
   sys_assert(res, "connection_set, connect");
   connptr[conn_num].fd       = client_fd; 

}

void Worker::connection_mod(connection* c)
{
    write_log("connection_mod, c = %p\n", c);
    struct epoll_event epv = {0, {0}};
    epv.events             = c->events;
    epv.data.ptr           = c;
    int res                = epoll_ctl(m_epfd, EPOLL_CTL_MOD, c->fd, &epv);
    sys_assert(res, "connection_mod, epoll_ctl");
}

void Worker::connection_del(int conn_num)
{
    connection conn    = m_connptr[conn_num];
    struct epoll_event epv = {0, {0}};
    int res                = epoll_ctl(m_epfd, EPOLL_CTL_DEL, conn.fd, &epv);
    sys_assert(res, "connection_del, epoll_ctl");
    close(conn.fd);
}
void Worker::reconnect(int conn_num)
{
    int events   = EPOLLOUT;
    int type     = m_connptr[conn_num].type;
    connection_del(conn_num);
    connection_add(conn_num, events, type);
}
void Worker::timeout_process()
{
    connection *connptr = m_connptr;
    struct timeval now;
    gettimeofday(&now, NULL);
    int conn_count = 0;
    int total_conn = m_info.get_num_connection();
    while (conn_count < total_conn) {
        if (connptr[conn_count].timeout.tv_sec < now.tv_sec && connptr[conn_count].timeout.tv_sec) {
	    reconnect(conn_count);
	    m_connptr[conn_count].timeout.tv_sec = 0;
	    write_log("timeout");
	    getchar();
	    getchar();
	}
	conn_count++;
    }
}
int Worker::get_timeout()
{
    return -1;
}

int Worker::add_timer(int timeout, int conn_num) 
{
    int count       = 0; 
    int total_conn  = m_info.get_num_connection();
    struct timeval now;
    gettimeofday(&now, NULL);
    while (count < conn_num && total_conn) {
        if (m_connptr[conn_num].timeout.tv_sec == 0) {
	    m_connptr[conn_num].timeout.tv_sec = now.tv_sec + timeout;
	    count++;
	}
	total_conn--;
    }
    return count;
}

int  Worker::add_connection(int conn_num, int type)
{
    int count         = 0;
    int current_num   = m_info.get_num_connection();
    int max_connection= m_info.get_max_connection();
    while (count < conn_num && count + current_num < max_connection) {
        connection_add(current_num + count, EPOLLOUT, type);
	count++;
    }
    m_info.set_num_connection(count + current_num);
    return count;
}
int Worker::command_process(int fd, int events, void *arg)
{
    connection *connp  = (connection*)arg;
    int command, target;
    unsigned int value1, value2;
    char buf_command[100];
    int res = read(fd, buf_command, sizeof(buf_command));
    buf_command[res] = '\0';
    write_log("pipe res = %d", res);
    sys_assert(res, "command_process, read");
    write_log("%s\n", buf_command);
    sscanf(buf_command, "%d:%d:%d:%d", &command, &target, &value1, &value2);
    write_log("%d:%d:%d:%d\n", command, target, value1, value2);
    unsigned int timeout, conn_num, type;
    switch (command) {
        case C_ADD:
            switch (target) {
	        case T_TIMER:
		    timeout   = value1;
		    conn_num  = value2;
		    add_timer(timeout, conn_num);
	            break;
	        case T_CONN:
		    type      = value1;
		    conn_num  = value2; 
		    res       = add_connection(conn_num, type);
		    write_log("add_connection = %d\n", res);
	            break;
	    }
        break;
    }  
    return S_OTHER;
}
void Worker::write_log(const char *fmt,...) 
{
#ifdef DEBUG
    char logbuf[100];
    va_list va;
    va_start(va, fmt);
    vsnprintf(logbuf, sizeof(logbuf), fmt, va);
    //printf("%s\n",logbuf);
    write(m_logfd, logbuf, strlen(logbuf));
    va_end(va);
#endif
}
