#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "message.h"


extern struct pthread_message_buf	message_buf[PTHREAD_MAX];

void pthreadA()
{
	while(1)
	{
        printf("waiting messag:\n");
        pthread_recv_message(&message_buf[PTHREAD_LED], PTHREAD_LED, true);
        printf("recv cnt = %d\n", message_buf[PTHREAD_LED].value);
	}
	
}

void pthreadB()
{
    u32 cnt = 0;
	while(1)
	{
		getchar();
        message_buf[PTHREAD_LED].value = cnt++;
        pthread_send_message(&message_buf[PTHREAD_LED], PTHREAD_LED);
	}
	
}
void pthreadC()
{
    u32 cnt = 0;
	while(1)
	{
        //sleep(2);
        //message_buf[PTHREAD_LED].value = cnt++;
        message_buf[PTHREAD_LED].value = 1;
        pthread_send_message(&message_buf[PTHREAD_LED], PTHREAD_LED);
	}
	
}
void pthreadD()
{
    u32 cnt = 0;
	while(1)
	{
        //sleep(1);
        //message_buf[PTHREAD_LED].value = cnt++;
        message_buf[PTHREAD_LED].value = 2;
        pthread_send_message(&message_buf[PTHREAD_LED], PTHREAD_LED);
	}
	
}

int main(void)
{
	pthread_t pid[2];
	
    pthread_init_message();
	
	pthread_create(&pid[0], NULL, (void *)pthreadA, NULL);
	//pthread_create(&pid[1], NULL, (void *)pthreadB, NULL);
	pthread_create(&pid[1], NULL, (void *)pthreadC, NULL);
	pthread_create(&pid[1], NULL, (void *)pthreadD, NULL);
	
	pthread_join(pid[0], NULL);
	pthread_join(pid[1], NULL);
    pthread_destroy_message();

	return 0;	
}

