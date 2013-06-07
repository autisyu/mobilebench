#include "call-back.h"
int CallbackA::StateProcess(int fd, int events, void *arg)
{
   connection *connp  = (connection*)arg;
   LOG(stderr, "state_process, fd = %d, events = %x, state = %d, connp = %p\n", fd, events, connp->state, connp);
   fd                 = connp->fd;
   char *buf          = connp->packet;
   int &state         = connp->state;
   int &event         = connp->events;
   int res;
   switch (state) {
       case S_W:
           if (events & EPOLLOUT) {
	       res = Write(fd, buf);
	   if (res >= 0) {
               state = S_R;
               event = EPOLLIN; 
               LOG(stderr, "state translate from S_W to S_R\n");
	       return S_SWITCH;
	   } else {
	       return S_ERROR;
	   }
	   }
	   break;
       case S_R:
           if (events & EPOLLIN) {
	       res = Read(fd, buf);
	   if (res >= 0) {
               //state = STATER;
               //event = EPOLLIN; 
               LOG(stderr, "state translate from S_R to S_O\n");
	       //return STATESWITCH;
               return S_OTHER;
	   } else {
	       return S_ERROR;
	   }
	   }
	   return 0;
	   break;
       case S_BOTH:
           if (events & EPOLLOUT) {
	       Read(fd, buf);
	   }
	   if (events & EPOLLIN) {
	       Write(fd, buf);
	   }
	   break;
   }
   return 0;
}
int CallbackA::Write(int fd, char *buf)
{
   int           res = send(fd, buf, strlen(buf), 0);
   sys_assert(res, "CallbackA::write, send");
   LOG(stderr, "send memssage len = %d, errno = %d\n", res, errno);
   return res;
}
int CallbackA::Read(int fd, char *buf)
{
   
   int           res = recv(fd, buf, 128, 0);
   sys_assert(res, "CallbackA::read, recv");
   LOG(stderr, "recv memssage len = %d, sizeof(buf) = %d\n", res, sizeof(buf));
   return res;
}
StateProcess_t CallbackFactory::ReturnCallback(int type)
{
    if (type < 0) { 
        return NULL;
    }
    switch (type) {
        case 0:
	    return CallbackA::StateProcess;
	    break;
    }
}
