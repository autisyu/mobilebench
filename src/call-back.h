#ifndef _CALLBACK_
#define _CALLBACK_
#include<sys/epoll.h>
#include<sys/socket.h>
#include <string.h>
#include "connection.h"
#include "util.h"
typedef int (*StateProcess_t)(int fd, int events, void* arg);
class Callback
{
    public:
        virtual int StateProcess(int fd, int events, void *arg) = 0;
    protected:
        virtual int Read(int fd, int events, void *arg)  = 0;
        virtual int Write(int fd, int events, void *arg) = 0;
};

class CallbackA
{
    public:
        int static StateProcess(int fd, int events, void *arg);
    private:
        int static Read(int fd, char* buf);
        int static Write(int fd, char* buf);
};

class CallbackFactory
{
    public:
         StateProcess_t static ReturnCallback(int type);
};
#endif
