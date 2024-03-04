
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>


void *myfun(void *arg)
{
    cpu_set_t mask;
    cpu_set_t get;
    char buf[256];

    int i,j;
    int num = sysconf(_SC_NPROCESSORS_CONF);

    printf("system has %d processor\n", num);

    for(i=0; i<num; i++)
    {
        CPU_ZERO(&mask);
        CPU_SET(num -1 - i, &mask);	
        if(pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
        {
            fprintf(stderr, "set thread affinity fail\n");	
        }
        CPU_ZERO(&mask);
        if(pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0)
        {
            fprintf(stderr, "set thread affinity fail\n");	
        }
        for(j=0; j<num; j++)
        {
            if(CPU_ISSET(j, &get))
            {
                printf("thread %d is running in processor %d\n", (int)pthread_self(), j);	
            }

        }
        while(j++ < 100000000)
        {
            memset(buf, 0, sizeof(buf));	
        }

    }
    pthread_exit(NULL);


}

int main()
{
    pthread_t tid;

    if(pthread_create(&tid, NULL, (void *)myfun, NULL) != 0)
    {
        fprintf(stderr, "thread create failed\n");	
        return -1;
    }
    pthread_join(tid, NULL);
    return 0;
}


#if 0
p = (struct _arg *)para;
alg_para = (ALG *)&p->alg_para;
i = p->num;


num = sysconf(_SC_NPROCESSORS_CONF);

printf("system has %d processor\n", num);

CPU_ZERO(&mask);
CPU_SET(i, &mask);	
if(pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) < 0)
{
    fprintf(stderr, "set thread affinity fail\n");	
}
CPU_ZERO(&mask);
if(pthread_getaffinity_np(pthread_self(), sizeof(get), &get) < 0)
{
    fprintf(stderr, "set thread affinity fail\n");	
}
for(j=0; j<num; j++)
{
    if(CPU_ISSET(j, &get))
    {
        printf("thread %d is running in processor %d\n", (int)pthread_self(), j);	
    }

}

#endif
