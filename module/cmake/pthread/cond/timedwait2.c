#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
 

#define EXIT_MSG      2
#define NOTICE_MSG    1
#define WAIT_MSG      0
#define IS_EXIT_MSG(flag)   (flag == EXIT_MSG)

int recv_ack = WAIT_MSG;
pthread_mutex_t  recv_ack_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t   recv_ack_cond = PTHREAD_COND_INITIALIZER;

typedef struct mutex_cond
{
		pthread_condattr_t cattr;
        pthread_mutex_t i_mutex;
        pthread_cond_t i_cv;
		void* i_sigevent; 
}mutex_cond_t;



int setcondattr(pthread_cond_t *i_cv)
{
    pthread_condattr_t cattr;
	int ret = pthread_condattr_init(&cattr);
	if (ret != 0){
		return (1);
	}
    //clock_gettime(CLOCK_REALTIME, &ts);
	ret = pthread_condattr_setclock(&cattr, CLOCK_REALTIME);
	ret = pthread_cond_init(i_cv, &cattr);
	return 0;
}

// g++ -o pwait pwait.cpp -lpthread -lrt
int main()
{
    int rc = 0;
    int send_repeat = 0;
    struct timespec ts;

    if(!setcondattr(&recv_ack_cond)){
        printf("setcondattr sucess!\n");
    }


    printf("get lock\n");
    //pthread_mutex_lock(&recv_ack_mutex);
    clock_gettime(CLOCK_REALTIME, &ts);
    //clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec += 2;
    rc = 0;
    while ((recv_ack != NOTICE_MSG) && rc == 0){
        printf("cond_wait\n");
        rc = pthread_cond_timedwait(&recv_ack_cond, &recv_ack_mutex, &ts);
        printf("rc == %d\n", rc);
    }
    if (rc == 0){
        recv_ack= WAIT_MSG;//clear
        printf("recv send ack..\n");
    }else if(rc == ETIMEDOUT){//timeout
        printf("recv ack timeout! cnt = %d\n", send_repeat);
    }else{
        printf("recv error! cnt = %d\n", send_repeat);
    }
    pthread_mutex_unlock(&recv_ack_mutex);
    if (rc == 0){
    }

#if 0

    pthread_mutex_lock(&recv_ack_mutex);
    clock_gettime(CLOCK_MONOTONIC, &tv);
    printf("now time:%ld\n", tv.tv_sec);
    tv.tv_sec += 2;// 设置20秒后没收到事件超时返回
    rc = 0;
    while ((recv_ack != NOTICE_MSG) && rc == 0)
    {
        rc = pthread_cond_timedwait(&recv_ack_cond, &recv_ack_mutex, &tv);
        printf("rc = %d-%d\n", rc, ETIMEDOUT);
        break;
    }
    if (rc == 0){
        recv_ack= WAIT_MSG;//clear
        printf("recv send ack..\n");
    }else if(rc == ETIMEDOUT){//timeout
        printf("recv ack timeout! cnt = %d\n", send_repeat);
    }else{
        printf("recv error! cnt = %d\n", send_repeat);
    }
    pthread_mutex_unlock(&recv_ack_mutex);
#endif
    return 0;
}

#if 0
    printf("get lock\n");
    pthread_mutex_lock(&(mcond.i_mutex));
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 2;
    rc = 0;
    while ((recv_ack != NOTICE_MSG) && rc == 0){
        printf("cond_wait\n");
		rc = pthread_cond_timedwait(&(mcond.i_cv), &(mcond.i_mutex), &ts);
        printf("rc == %d\n", rc);
    }
    if (rc == 0){
        recv_ack= WAIT_MSG;//clear
        printf("recv send ack..\n");
    }else if(rc == ETIMEDOUT){//timeout
        printf("recv ack timeout! cnt = %d\n", send_repeat);
    }else{
        printf("recv error! cnt = %d\n", send_repeat);
    }
    pthread_mutex_unlock(&(mcond.i_mutex));

	return 0;
#endif





