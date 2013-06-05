#include "call-back.h"
void CallbackA::state_process(int fd, int events, void *arg)
{
   connection *connp = (connection*)arg;
   fd                = connp->fd;
   char *buf         = connp->packet;
   int state         = connp->state;
   switch (state) {
       case STATEW:
           if (events & EPOLLIN) {
	       write(fd, buf);
	   }
	   break;
       case STATER:
           if (events & EPOLLOUT) {
	       read(fd, buf);
	   }
   }
}
void CallbackA::write(int fd, char *buf)
{
   int           res = send(fd, buf, strlen(buf), 0);
   sys_assert(res, "CallbackA::write, send");
}
void CallbackA::read(int fd, char *buf)
{
   int           res = recv(fd, buf, sizeof(buf), 0);
   sys_assert(res, "CallbackA::read, recv");
}
