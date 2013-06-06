#ifndef _CONNECTION_
#define _CONNECTION_
#include<sys/time.h>
#include "call-back.h"
#include <errno.h>
#define UNCONNECT   0
#define S_1       1
#define S_2       2
#define S_R       3
#define S_W       4
#define S_BOTH    5 
#define S_ERROR   6
#define S_SWITCH  7 
#define S_OTHER   8
#define C_ADD     1
#define T_TIMER   1
#define T_CONN    2
class connection
{
    public:
        int conn_id;
        int fd;
	int state;
        int events;
	int type;
        //int time;
        struct timeval timeout;
        char packet[128];
        void *arg;
        int (*call_back)(int fd, int events, void *arg);
	//Callback call_back;
};
#endif/*_CONNECTION_*/
