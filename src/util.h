#ifndef _UTIL_
#define _UTIL_
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#ifdef DEBUG
#define LOG fprintf
#else
#define LOG
#endif
void static inline sys_assert(int res, const char *sys_name) 
{
    if (res == -1) {
	perror(sys_name);
	exit(-1);
    }
}

void static inline res_assert(void *res, const char *name) 
{
    if (res == NULL) {
        perror(name);
        //fprintf(stderr, "%s error!\n", name);
	exit(-1);
    }
}
#endif/*_UTIL_*/
