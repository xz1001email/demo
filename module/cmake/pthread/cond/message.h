/*****************************File Description****************************
* Main Functions:thread message function
* Author: ysy@mail.ie.ac.cn
* Modification Record:
*		Version		name			content
*		V1.0	  yangshaoying		create this file
* Note:
*************************************************************************/
#ifndef MESSAGE_H
#define MESSAGE_H

//#include "define.h"


#define bool char
#define true 1
#define false 0

#define FAILURE false
#define SUCCESS true


#define err(format,...) printf(format, ##__VA_ARGS__)
#define MY_DEBUG(format,...) printf(format, ##__VA_ARGS__)


typedef unsigned int u32;
typedef unsigned short int u16;
typedef signed int s32;
typedef signed short int s16;


#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */


//系统各线程ID分配
enum pthread_id {
	PTHREAD_MAIN = 1,
	PTHREAD_LED,
	PTHREAD_GPS,
	PTHREAD_MONITOR,
	PTHREAD_INPUT,
	PTHREAD_DISK,
	PTHREAD_NETCMD,
	PTHREAD_NETDATA,
	PTHREAD_MAX
}; 
enum pthread_message_type {
	MSG_SHOW_GPS,
	MSG_SETUP,
	MSG_CSAMT_SETUP,
	MSG_MAX
}; //模块收到的消息

struct pthread_message_buf { 
	enum pthread_id start_id; 	//发送消息的模块id
	u16 type; 					//消息类型
	u32 value; 				//消息类型对应的消息值
};

enum message_flag_t {
	WRITE_FLAG,
	READ_FLAG
}; //各线程消息的控制

bool pthread_send_message(struct pthread_message_buf *buf, enum pthread_id id); 
bool pthread_recv_message(struct pthread_message_buf *buf, enum pthread_id id, bool wait); 
s32  pthread_init_message(void);
s32  pthread_destroy_message(void);



#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */


#endif
