#include "worker.h"
#include "util.h"
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>

static int process();
int worker            = 1;
int num_of_request    = 1;
int request_per_worker= num_of_request / worker;
int stop              = 0;
const char *server_config[]   = {"192.168.106.202", "1234"};
int main(int argc, char **agrv)
{
    process();
    return 0;
}
int process()
{
    int port  = atoi(server_config[1]);
    int count = 0;
    pid_t  pid[10];
    Worker *w = NULL;
    while (count < worker) {
        pid[count] = fork();
        switch(pid[count]) {
	    case -1:
	        sys_assert(pid[count], "fork");
	        break;
	    case  0:
	        LOG(stderr, "%s", "I am child\n");
	        w = new Worker(request_per_worker, server_config[0], port);
		res_assert((void*)w, "process");
		w->start();
	        break;
	    default:
	        printf("success child %d\n", count);
	}
	count++;
    } 
    int status;
    for (count = 0; count < worker; ++count) {
        waitpid(pid[count], &status, 0);
    }

}
