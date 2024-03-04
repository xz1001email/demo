#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>



int record_run_time()
{
    char logbuf[200];
    time_t t;
    struct tm *tmp;
    struct timeval tv; 
    uint32_t us;

    //t = time(NULL);
    gettimeofday(&tv, NULL);
    t = tv.tv_sec; 
    us = tv.tv_usec; 
    tmp = localtime(&t);
    if (tmp == NULL) {
        perror("localtime");
        return -1;
    }

//#define STR_FORMAT  "run time: %A %a %B %b %C %c"
#define STR_FORMAT  "run time: %F %T"

    if (strftime(logbuf, sizeof(logbuf), STR_FORMAT, tmp) == 0) {
        fprintf(stderr, "strftime returned 0");
        return -1;
    }

    printf("Result string: %s, us: %d\n", logbuf, us);
    return 0;
}


int main()
{
    record_run_time();
    return 0;
}
