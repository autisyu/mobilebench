#ifndef _CALLBACK_
#define _CALLBACK_
#include<sys/epoll.h>
#include<sys/socket.h>
#include "connection.h"
#include "util.h"
#include <string.h>
class Callback
{
    public:
        virtual int state_process(int fd, int events, void *arg) = 0;
    protected:
        virtual int read(int fd, int events, void *arg)  = 0;
        virtual int write(int fd, int events, void *arg) = 0;
};

class CallbackA
{
    public:
        int static state_process(int fd, int events, void *arg);
    private:
        int static read(int fd, char* buf);
        int static write(int fd, char* buf);
};
#endif
