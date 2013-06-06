#include"worker.h"
Worker::Worker(const char* serv_ip, int serv_port)
{
    int max_connection = 70000;
    int num_connection = 0;
    m_info.set_num_connection(num_connection);
    m_info.set_max_connection(max_connection);
    ConfigServer(serv_ip, serv_port);
    m_events         = new epoll_event[max_connection];
    res_assert(m_events, "new epoll_event\n");

    m_connptr        = new connection[max_connection];
    res_assert(m_connptr, "new connection\n");

    m_epfd           = epoll_create(num_connection + 1);
    sys_assert(m_epfd, "epoll_create");
}

void Worker::StartLog()
{
    char log_file_name[100];
    sprintf(log_file_name, "%d_worker_log", m_info.get_id());
    m_logfd            = open(log_file_name, O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (-1 == m_logfd) {
        perror("fopen");
	exit(-1);
    } else {
        dup2(m_logfd, STDERR_FILENO);
    }

}
void Worker::ConfigServer(const char *serv_ip, int serv_port)
{
    m_server_port = serv_port;
    memmove(m_server_ip, serv_ip, strlen(serv_ip));
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
void Worker::AddMonitor()
{
    m_monitor.fd        = m_info.get_rpipe();
    //m_monitor.call_back = Worker::CommandProcess;
    epoll_event epv     = {0, {0}};
    epv.events          = EPOLLIN;
    epv.data.ptr        = &m_monitor;
    epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_monitor.fd, &epv);
}
void Worker::Start()
{
    AddMonitor();
    StartLog();
    DoWork();
}

void Worker::DoWork()
{
    Dispatch();
}

void Worker::Dispatch()
{
    long time_out = -1;
    int res, i, rstatus;
    while (1) {
        //WriteLog("start event loop, m_epfd = %d, m_events = %p, max_conn = %d\n", m_epfd, m_events, m_info.get_max_connection());
        time_out        = GetTimeout();
	WriteLog("timeout will happen at %d\n", time_out);
        res             = epoll_wait(m_epfd, m_events, m_info.get_max_connection(), time_out * 1000);
	sys_assert(res, "dispatch, epoll_wait");
	WriteLog("return event = %d\n", res);
	for (i = 0; i < res; ++i) {
	    connection *c   = (connection*)m_events[i].data.ptr; 
	    if (c == &m_monitor) {
	        CommandProcess(c->fd, m_events[i].events, c->arg);
		continue;
	    }
	    //WriteLog("return fd= %d, call_back = %p\n", c->fd, c->call_back);
	    rstatus         = c->call_back(c->fd,m_events[i].events, c->arg);
	    switch (rstatus) {
	        case S_SWITCH:
		    //WriteLog("STATESWITCH\n");
	            ConnectionMod(c);
		    break;
		case S_ERROR:
		    Reconnect(c->conn_id);
		    break;
	    }
	}
        WriteLog("end event loop\n");
	TimeoutProcess();
    }
}

void Worker::ConnectionAdd(int conn_num, int events, int type)
{
   WriteLog("add conn_num = %d\n", conn_num);
   struct connection *connptr = m_connptr;
   ConnectionSet(conn_num, events, type);
   struct epoll_event epv = {0, {0}};
   epv.data.ptr           = &connptr[conn_num];
   epv.events             = connptr[conn_num].events ;
   int res                = epoll_ctl(m_epfd, EPOLL_CTL_ADD, connptr[conn_num].fd, &epv);
   sys_assert(res, "ConnectionAdd, epoll_ctl");
}

void Worker::ConnectionSet(int conn_num, int events, int type)
{
   struct timeval now;
   struct connection *connptr        = m_connptr; 
   gettimeofday(&now, NULL);
   int client_fd                     = socket(AF_INET, SOCK_STREAM, 0);
   //fcntl(client_fd, F_SETFL, O_NONBLOCK);
   sys_assert(client_fd, "ConnectionSet, socket");
   //int timeout                = rand() % 4;
   strcpy(connptr[conn_num].packet, "hello world");
   connptr[conn_num].conn_id         = conn_num;
   //connptr[conn_num].time     = timeout;
   //now.tv_sec                 = timeout;
   connptr[conn_num].timeout.tv_sec  = 0; 
   connptr[conn_num].events          = events;
   connptr[conn_num].state           = S_W;
   connptr[conn_num].call_back       = CallbackA::state_process;
   connptr[conn_num].type            = type;
   connptr[conn_num].arg             = &connptr[conn_num];
   
   int res                           = connect(client_fd, (struct sockaddr*)&m_server_addr, sizeof(m_server_addr));
   /*int retry                  = 0;
   while (-1 ==(res= connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)))) {
       retry++;
   }*/

   //WriteLog(m_logfd, "connect res = %d, client_fd = %d, retry = %d\n", res, client_fd, retry);
   sys_assert(res, "ConnectionSet, connect");
   connptr[conn_num].fd              = client_fd; 

}

void Worker::ConnectionMod(connection* c)
{
    WriteLog("ConnectionMod, c = %p\n", c);
    struct epoll_event epv = {0, {0}};
    epv.events             = c->events;
    epv.data.ptr           = c;
    int res                = epoll_ctl(m_epfd, EPOLL_CTL_MOD, c->fd, &epv);
    sys_assert(res, "ConnectionMod, epoll_ctl");
}

void Worker::ConnectionDel(int conn_num)
{
    connection conn    = m_connptr[conn_num];
    struct epoll_event epv = {0, {0}};
    int res                = epoll_ctl(m_epfd, EPOLL_CTL_DEL, conn.fd, &epv);
    sys_assert(res, "ConnectionDel, epoll_ctl");
    close(conn.fd);
}
void Worker::Reconnect(int conn_num)
{
    int events   = EPOLLOUT;
    int type     = m_connptr[conn_num].type;
    ConnectionDel(conn_num);
    ConnectionAdd(conn_num, events, type);
}
void Worker::TimeoutProcess()
{
    connection *connptr = m_connptr;
    struct timeval now;
    gettimeofday(&now, NULL);
    WriteLog("wake up at %d\n", now.tv_sec);
    if (m_heap_timer.empty()) { return ;}
    if (now.tv_sec < m_heap_timer.top()) { return;} 
    m_heap_timer.pop();
    WriteLog("timeout at %d\n", now.tv_sec);
    int conn_count = 0;
    int total_conn = m_info.get_num_connection();
    while (conn_count < total_conn) {
        WriteLog("now.tv_sec = %d, target.tv_sec = %d\n", now.tv_sec, connptr[conn_count].timeout.tv_sec);
        if (connptr[conn_count].timeout.tv_sec <= now.tv_sec && connptr[conn_count].timeout.tv_sec) {
	    Reconnect(conn_count);
	    m_connptr[conn_count].timeout.tv_sec = 0;
	    WriteLog("timeout, count = %d\n", conn_count);
	}
	conn_count++;
    }
}
int Worker::GetTimeout()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    int res = -1;
    if (!m_heap_timer.empty()) {
        res = m_heap_timer.top() - now.tv_sec; 
	//m_heap_timer.pop();
	if (res < 0) {
	    TimeoutProcess();
	    GetTimeout();
	} else {
	    return res;
	}
    }
    return res;
}

int Worker::AddTimer(int timeout, int conn_num) 
{
    int count       = 0; 
    int pos         = 0;
    int total_conn  = m_info.get_num_connection();
    struct timeval now;
    gettimeofday(&now, NULL);
    WriteLog("AddTimer at %d, timeout = %d, conn_num = %d\n", now.tv_sec, timeout, conn_num);
    m_heap_timer.push(now.tv_sec + timeout);
    while (count < conn_num && pos < total_conn) {
        if (m_connptr[pos].timeout.tv_sec == 0) {
	    m_connptr[pos].timeout.tv_sec = now.tv_sec + timeout;
	    count++;
	}
	pos++;
    }
    WriteLog("AddTimer, total = %d\n", count);
    return count;
}

int  Worker::AddConnection(int conn_num, int type)
{
    int count         = 0;
    int current_num   = m_info.get_num_connection();
    int max_connection= m_info.get_max_connection();
    while (count < conn_num && count + current_num < max_connection) {
        ConnectionAdd(current_num + count, EPOLLOUT, type);
	count++;
    }
    m_info.set_num_connection(count + current_num);
    return count;
}
int Worker::CommandProcess(int fd, int events, void *arg)
{
    connection *connp  = (connection*)arg;
    int command, target;
    unsigned int value1, value2;
    char buf_command[100];
    int res = read(fd, buf_command, sizeof(buf_command));
    buf_command[res] = '\0';
    WriteLog("pipe res = %d", res);
    sys_assert(res, "CommandProcess, read");
    WriteLog("%s\n", buf_command);
    sscanf(buf_command, "%d:%d:%d:%d", &command, &target, &value1, &value2);
    WriteLog("%d:%d:%d:%d\n", command, target, value1, value2);
    unsigned int timeout, conn_num, type;
    switch (command) {
        case C_ADD:
            switch (target) {
	        case T_TIMER:
		    timeout   = value1;
		    conn_num  = value2;
		    AddTimer(timeout, conn_num);
	            break;
	        case T_CONN:
		    type      = value1;
		    conn_num  = value2; 
		    res       = AddConnection(conn_num, type);
		    WriteLog("AddConnection = %d\n", res);
	            break;
	    }
        break;
    }  
    return S_OTHER;
}
void Worker::WriteLog(const char *fmt,...) 
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
