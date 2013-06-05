#ifndef _CONNECTION_
#define _CONNECTION_
#include<sys/time.h>
#include<call-back.h>
class connection
{
    int fd;
    int events;
    int time;
    struct itimerval timeout;
    char packet[128];
    void *arg;
    void (*call_back)(int fd, int events, void *arg);
    Callback cb;
};
#endif/*_CONNECTION_*/
