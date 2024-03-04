#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
 
typedef struct mutex_cond
{
		pthread_condattr_t cattr;
        pthread_mutex_t i_mutex;
        pthread_cond_t i_cv;
		void* i_sigevent; 
}mutex_cond_t;
 
// g++ -o pwait pwait.cpp -lpthread -lrt
int main()
{
	mutex_cond_t mcond;
	int ret = pthread_condattr_init(&(mcond.cattr));
	if (ret != 0)
	{
		return (1);
	}
	mcond.i_sigevent = NULL;
    ret = pthread_mutex_init ( &(mcond.i_mutex), NULL);
	ret = pthread_condattr_setclock(&(mcond.cattr), CLOCK_MONOTONIC);
	ret = pthread_cond_init(&(mcond.i_cv), &(mcond.cattr));
 
	struct timespec tv;
	while(1)
	{
		clock_gettime(CLOCK_MONOTONIC, &tv);
		printf("now time:%ld\n", tv.tv_sec);
		tv.tv_sec += 2;// 设置20秒后没收到事件超时返回
		ret = pthread_cond_timedwait(&(mcond.i_cv), &(mcond.i_mutex), &tv);
        printf("ret = %d-%d\n", ret, ETIMEDOUT);
	}
	
	return 0;
}


