#ifndef _CONNECTION_
#define _CONNECTION_
#include<sys/time.h>
#include "call-back.h"
class connection
{
    public:
        int fd;
	int state;
        int events;
        int time;
        struct timeval timeout;
        char packet[128];
        void *arg;
        void (*call_back)(int fd, int events, void *arg);
	//Callback call_back;
};
#endif/*_CONNECTION_*/
