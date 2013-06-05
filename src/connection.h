#ifndef _CONNECTION_
#define _CONNECTION_
#include<sys/time.h>
#include "call-back.h"
#include <errno.h>
#define UNCONNECT   0
#define STATT1      1
#define STATT2      2
#define STATER      3
#define STATEW      4
#define STATEBOTH   5 
#define STATEERROR  6
#define STATESWITCH 7
class connection
{
    public:
        int conn_id;
        int fd;
	int state;
        int events;
        int time;
        struct timeval timeout;
        char packet[128];
        void *arg;
        int (*call_back)(int fd, int events, void *arg);
	//Callback call_back;
};
#endif/*_CONNECTION_*/
