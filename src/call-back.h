#ifndef _CALLBACK_
#define _CALLBACK_
#define UNCONNECT 0
#define STATT1    1
#define STATT2    2
#define STATER    3
#define STATEW    4
#include<sys/epoll.h>
#include<sys/socket.h>
#include "connection.h"
#include "util.h"
#include <string.h>
class Callback
{
    public:
        virtual void state_process(int fd, int events, void *arg) = 0;
    protected:
        virtual void read(int fd, int events, void *arg)  = 0;
        virtual void write(int fd, int events, void *arg) = 0;
};

class CallbackA
{
    public:
        void static state_process(int fd, int events, void *arg);
    private:
        void static read(int fd, char* buf);
        void static write(int fd, char* buf);
};
#endif
