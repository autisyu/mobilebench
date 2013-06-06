#include "call-back.h"
int CallbackA::state_process(int fd, int events, void *arg)
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
	       res = write(fd, buf);
	   if (res >= 0) {
               state = S_R;
               event = EPOLLIN; 
	       return S_SWITCH;
	   } else {
	       return S_ERROR;
	   }
	   }
	   break;
       case S_R:
           if (events & EPOLLIN) {
	       res = read(fd, buf);

	   if (res > 0) {
               //state = STATER;
               //event = EPOLLIN; 
	       //return STATESWITCH;
	   } else {
	       return S_ERROR;
	   }
	   }
	   return 0;
	   break;
       case S_BOTH:
           if (events & EPOLLOUT) {
	       read(fd, buf);
	   }
	   if (events & EPOLLIN) {
	       write(fd, buf);
	   }
	   break;
   }
   return 0;
}
int CallbackA::write(int fd, char *buf)
{
   int           res = send(fd, buf, strlen(buf), 0);
   sys_assert(res, "CallbackA::write, send");
   LOG(stderr, "send memssage len = %d, errno = %d\n", res, errno);
   return res;
}
int CallbackA::read(int fd, char *buf)
{
   
   int           res = recv(fd, buf, sizeof(buf), 0);
   sys_assert(res, "CallbackA::read, recv");
   LOG(stderr, "recv memssage len = %d\n", res);
   return res;
}

