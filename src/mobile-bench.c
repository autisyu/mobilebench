#include "worker.h"
#include "util.h"
#include<sys/types.h>
#include<sys/wait.h>
#include<stdio.h>
#include<getopt.h>
#include<stdlib.h>

static int Process(int worker);
static int UserMoniter();
int worker            = 2;
int num_of_request    = 10;
int request_per_worker= num_of_request / worker;
int stop              = 0;
const char *server_config[]   = {"192.168.106.112", "12345"};
int pipe_fd[10][2];
char*server_ip;
int server_port;

static void Usage()
{
    fprintf(stderr,"mobile-bench -h hostname -p hostport -w workernum\n");
    errno = EINVAL;
    exit(-1);
}
static void MonitorUsage()
{
    printf("command line, you can only use number like action:target:value1:value2\n\ 
           action :\n\
           ADD    :1\n\
           target :\n\
           TIMER  :1\n\
           CONN   :2\n\
           value1 :\n\
           timeout:(0-maxint)\n\
           contype:(0-maxint)\n\
           value2 :\n\
           affconn:(1-connection you have set)\n\
           1:2:1:100 means you add 100 type 1 connection\n");
}
static int ParseInput(int argc, char **argv)
{
    int opt, options_index;
    while ((opt = getopt_long(argc, argv, "h:p:w:", NULL, &options_index)) != EOF) {
        switch (opt) {
            case 'h':
                server_ip   = optarg;
                break;
            case 'p':
                server_port = atoi(optarg);
                break;
            case 'w':
                worker      = atoi(optarg); 
        }

    } 
}
int main(int argc, char **argv)
{
    if (argc != 7) {
        Usage();
    }
    if (worker > 10) {
        fprintf(stderr, "too much worker you choose, we reset it to 10\n");
        worker = 10;
    }
    ParseInput(argc, argv);
    Process(worker);
    return 0;
}

int Process(int worker)
{
    //int port  = atoi(server_config[1]);
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
	        w = new Worker(server_ip, server_port);
		res_assert((void*)w, "process");
		w->m_info.set_rpipe(pipe_fd[count][0]);
		w->m_info.set_id(count);
		close(pipe_fd[count][1]);
		w->Start();
	        break;
	    default:
		close(pipe_fd[count][0]);
	        count++;
	        printf("success child %d\n", count);
	}
    } 
    if (pid[count - 1]) {
        UserMoniter();
        for (count = 0; count < worker; ++count) {
            waitpid(pid[count], &status, 0);
        }
        LOG(stderr, "child all stop\n");
    }
}
static int UserMoniter()
{
    char command[100];
    int action = -1, target = -1, value1 = -1, value2 = -1, value;
    int con_per_worker;
    int res;
    char c;
    MonitorUsage();
    while (1) {
        int count = 0;
	printf("user_moniter>>");
        scanf("%s", command);
	sscanf(command,"%d:%d:%d:%d",&action, &target, &value1, &value2);
	if (action == -1 || target == -1 || value1 == -1 || value2 == -1) {
	    MonitorUsage();
	    continue;
	}
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
