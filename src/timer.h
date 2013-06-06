#ifndef _TIMER_
#define _TIMER_
#include<sys/time.h>
class Timer
{
    public:
        int id;
	struct timeval timeout;
        int num_of_connection;
        int action;
};
#endif/*_TIMER_*/
