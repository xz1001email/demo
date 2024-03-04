#include    <stdio.h>  
#include    <stdlib.h>  
#include    <unistd.h>  
#include    <pthread.h>  
#include    <signal.h>  
#include    <string.h>  
void * thr_fun(void *arg)  
{  
    int policy, ret;  
    struct sched_param param;  
    //获取线程调度参数  
    ret = pthread_getschedparam(pthread_self(), &policy, &param);  
    if(ret!=0)  
    {  
        printf("pthread_getschedparam %s/n", strerror(ret) );  
        exit(1);  
    }  
    if (policy == SCHED_FIFO)  
    {  
        printf("policy:SCHED_FIFO/n");  
    }  
    else if (policy == SCHED_OTHER)  
    {  
        printf("policy:SCHED_OTHER/n");  
    }  
    else if (policy == SCHED_RR)  
    {  
        printf("policy:SCHED_RR/n");  
    }  
    printf("param:%d/n", param.sched_priority);   
    long long i;  
    while (1) {  
        i++;  
        i *= 2;  
    }  
    pthread_exit(NULL);   
}  
int main(void)  
{  
    int ret;  
    pthread_t tid;  
    pthread_attr_t attr;  
    int policy, inher;  
    struct sched_param param;  
      
    //初始化线程属性  
    pthread_attr_init(&attr);  
    //获取继承的调度策略  
    ret = pthread_attr_getinheritsched(&attr, &inher);  
    if (ret!=0)  
    {  
        printf("pthread_attr_getinheritsched/n%s/n", strerror(ret));  
        exit(1);  
    }  
    //  
    if (inher == PTHREAD_EXPLICIT_SCHED)   
    {  
        printf("PTHREAD_EXPLICIT_SCHED/n");  
    }  
    else if (inher == PTHREAD_INHERIT_SCHED)   
    {     
        printf("PTHREAD_INHERIT_SCHED/n");  
        inher = PTHREAD_EXPLICIT_SCHED;  
    }  
    //设置继承的调度策略  
    //必需设置inher的属性为 PTHREAD_EXPLICIT_SCHED，否则设置线程的优先级会被忽略  
    ret = pthread_attr_setinheritsched(&attr, inher);  
    if (ret!=0)  
    {  
        printf("pthread_attr_setinheritsched/n%s/n", strerror(ret));  
        exit(1);  
    }  
      
    policy = SCHED_FIFO;//在Ubuntu9.10上需要root权限  
    //设置线程调度策略  
    ret = pthread_attr_setschedpolicy(&attr, policy);  
    if (ret!=0)  
    {  
        printf(" pthread_attr_setschedpolicy/n%s/n", strerror(ret));  
        exit(1);  
    }  
    param.sched_priority = 3;  
    //设置调度参数  
    ret = pthread_attr_setschedparam(&attr, &param);  
    if (ret!=0)  
    {  
        printf(" pthread_attr_setschedparam/n%s/n", strerror(ret));  
        exit(1);  
    }  
    //创建线程  
    ret = pthread_create(&tid, &attr, thr_fun, NULL);  
    if (ret!=0)  
    {  
        printf("pthread_create/n%s/n", strerror(ret));  
        exit(1);  
    }  
    while (1) {  
        printf("hello world\n");  
    }  
    pthread_join(tid, NULL);  
    pthread_exit(NULL);  
} 
