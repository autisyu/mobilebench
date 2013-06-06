#include "worker.h"
#include "util.h"
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<stdlib.h>

static int process(int worker);
static int user_moniter();
int worker            = 2;
int num_of_request    = 10;
int request_per_worker= num_of_request / worker;
int stop              = 0;
const char *server_config[]   = {"192.168.106.202", "1234"};
int pipe_fd[10][2];
int main(int argc, char **agrv)
{
    process(worker);
    return 0;
}

int process(int worker)
{
    int port  = atoi(server_config[1]);
    int count = 0;
    pid_t  pid[10], pid_monitor;
    int res, status;
    Worker *w = NULL;
    while (count < worker) {
        res        = pipe(pipe_fd[count]);
	sys_assert(res, "pipe");
        pid[count] = fork();
        switch(pid[count]) {
	    case -1:
	        sys_assert(pid[count], "fork");
	        break;
	    case  0:
	        //LOG(stderr, "%s", "I am child\n");
	        w = new Worker(server_config[0], port);
		res_assert((void*)w, "process");
		w->m_info.set_rpipe(pipe_fd[count][0]);
		w->m_info.set_id(count);
		//printf("child count = %d\n", count);
		close(pipe_fd[count][1]);
		w->start();
	        break;
	    default:
		close(pipe_fd[count][0]);
	        count++;
	        printf("success child %d\n", count);
	}
    } 
    if (pid[count - 1]) {
        user_moniter();
        for (count = 0; count < worker; ++count) {
            waitpid(pid[count], &status, 0);
        }
        LOG(stderr, "child all stop\n");
    }
}
int user_moniter()
{
    char command[100];
    int action, target, value1, value2, value;
    int con_per_worker;
    int res;
    while (1) {
        int count = 0;
	printf("user_moniter>>");
        scanf("%s", command);
	//printf("user_moniter>>%s\n", command);
	sscanf(command,"%d:%d:%d:%d",&action, &target, &value1, &value2);
	con_per_worker = value2 / worker;
	snprintf(command, sizeof(command), "%d:%d:%d:%d", action, target, value1, con_per_worker);
	while (count < worker - 1) {
	    res = write(pipe_fd[count][1],command, strlen(command));
	    sys_assert(res, "write, user_moniter");
	    count++;
	}
	snprintf(command, sizeof(command), "%d:%d:%d:%d", action, target, value1, value2 - (worker - 1) * con_per_worker);
	res = write(pipe_fd[count][1],command, strlen(command));
	sys_assert(res, "write, user_moniter");
    }
}
