#ifndef _UTIL_
#define _UTIL_
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#ifdef DEBUG
#define LOG fprintf
#else
#define LOG
#endif
void static inline sys_assert(int res, const char *sys_name) 
{
    if (res == -1) {
        fprintf(stderr, "%s error!\n", sys_name);
    }
}

void static inline res_assert(void *res, const char *name) 
{
    if (res == NULL) {
        fprintf(stderr, "%s error!\n", name);
    }
}
#endif/*_UTIL_*/
