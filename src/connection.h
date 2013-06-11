#ifndef _CONNECTION_
#define _CONNECTION_
#include "call-back.h"
#include<sys/time.h>
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
	int data_len, offset;
        void *arg;
        int (*StateProcess)(int fd, int events, void *arg);
	//Callback call_back;
};

typedef struct 
{
    int action;
    int target;
    int param1;
    int param2; 
}command_t;

static void SetCommand(command_t &cmd, int action, int target, int param1, int param2)
{
    cmd.action = action;
    cmd.target = target;
    cmd.param1 = param1;
    cmd.param2= param2;
}
#endif/*_CONNECTION_*/
