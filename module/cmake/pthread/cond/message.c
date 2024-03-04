/*****************************File Description****************************
* Functions:
* Author: ysy@mail.ie.ac.cn
* Modification Record:
*		Version			name			content
*		v1.0.0		 yang shaoying	 creat this file
* Note:
*************************************************************************/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>

//#include "define.h"

#include "message.h"

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */



static inline s32 pthread_destroy_cond(void);
static inline s32 pthread_initialize_cond(void);
static inline s32 pthread_destory_mutex(void);
static inline s32 pthread_initialize_mutex(void);

pthread_mutex_t				message_mutex[PTHREAD_MAX];
pthread_cond_t				message_cond[PTHREAD_MAX];
enum message_flag_t			message_flag[PTHREAD_MAX];
struct pthread_message_buf	message_buf[PTHREAD_MAX];

/*************************************************************
Function: pthread_send_message()
Input: 
Output: 
Return: 
Note: 
*************************************************************/
bool pthread_send_message(struct pthread_message_buf *buf, enum pthread_id id)
{
	
	pthread_mutex_lock(&message_mutex[id]);
	while (message_flag[id] == WRITE_FLAG){
		pthread_cond_wait(&message_cond[id], &message_mutex[id]);
	}	
	
	message_flag[id] = WRITE_FLAG;
	memcpy(&message_buf[id], buf, sizeof(struct pthread_message_buf));
	pthread_mutex_unlock(&message_mutex[id]);
	pthread_cond_broadcast(&message_cond[id]);	

	return true;
} 
/*************************************************************
Function: pthread_recv_message()
Input: 
Output: 
Return: 
Note: 
*************************************************************/
bool pthread_recv_message(struct pthread_message_buf *buf, enum pthread_id id, bool wait)
{
	
	if ((wait == false)&&(message_flag[id] != WRITE_FLAG)){
		return false;
	}
	pthread_mutex_lock(&message_mutex[id]);
	while (message_flag[id] != WRITE_FLAG){
		if (wait == true){
			pthread_cond_wait(&message_cond[id], &message_mutex[id]);
		}else{
			pthread_mutex_unlock(&message_mutex[id]);
			pthread_cond_signal(&message_cond[id]);	
			return false;
		}		
	}
	message_flag[id] = READ_FLAG;
	memcpy(buf, &message_buf[id], sizeof(struct pthread_message_buf));

	pthread_mutex_unlock(&message_mutex[id]);
	pthread_cond_signal(&message_cond[id]);
	
	return true;
}
/*************************************************************
Function: pthread_init_message()
Input: 
Output: 
Return: 
Note: 
*************************************************************/
s32 pthread_init_message(void)
{
	u32 i;
	s32 ret;
	
	ret=pthread_initialize_cond();
	if (ret < 0){
		return FAILURE;
	}

	ret=pthread_initialize_mutex();
	if (ret < 0){
		return FAILURE;
	}

	for(i=0; i < PTHREAD_MAX; i++){
		message_flag[i]=READ_FLAG;
	}

	return SUCCESS;
}
/*************************************************************
Function: pthread_destroy_message()
Input: 
Output: 
Return: 
Note: 
*************************************************************/
s32 pthread_destroy_message(void)
{
	s32 ret;
	ret=pthread_destory_mutex();
	if (ret < 0){
		return FAILURE;
	}

	ret=pthread_destroy_cond();
	if (ret < 0){
		return FAILURE;
	}

	return SUCCESS;
}
/*************************************************************
Function: pthread_initialize_mutex()
Input: 
Output: 
Return: 
Note: 
*************************************************************/
static inline s32 pthread_initialize_mutex(void)
{
	int i;
	
	for(i=0; i < PTHREAD_MAX; i++) {
		if ( (pthread_mutex_init(&message_mutex[i], NULL)) != 0 ) {
			err("\t!!! ERROR : pthread_mutex_init--%s !!!\n",strerror(errno));
			return FAILURE;	    	
		}
	}
	
	return SUCCESS;
}
/*************************************************************
Function: pthread_destory_mutex()
Input: 
Output: 
Return: 
Note: 
*************************************************************/
static inline s32 pthread_destory_mutex(void)
{
	int i;
	
	for(i=0; i < PTHREAD_MAX; i++) {
		if ( (pthread_mutex_destroy(&message_mutex[i])) != 0 ) {
			err("\t!!! ERROR : pthread_mutex_destroy--%s !!!\n",strerror(errno));
			return FAILURE;		    	
		}
	}
	
	return SUCCESS;
}
/*************************************************************
Function: pthread_initialize_cond()
Input: 
Output: 
Return: 
Note: 
*************************************************************/
static inline s32 pthread_initialize_cond(void)
{
	int i;
	
	for(i=0; i < PTHREAD_MAX; i++) {
		if ( (pthread_cond_init(&message_cond[i], NULL)) != 0 ) {
			err("\t!!! ERROR : pthread_cond_init--%s !!!\n",strerror(errno));
			return FAILURE;	 		
		}
	}
	
	return SUCCESS;
}

/*************************************************************
Function: pthread_destroy_cond()
Input: 
Output: 
Return: 
Note: 
*************************************************************/
static inline s32 pthread_destroy_cond(void)
{
	int i;
	
	for(i=0; i < PTHREAD_MAX; i++) {
		if ( (pthread_cond_destroy(&message_cond[i])) != 0 ) {
			err("\t!!! ERROR : pthread_cond_destroy--%s !!!\n",strerror(errno));
			return FAILURE;
		}	 	
	}
	
	return SUCCESS;
}


#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

