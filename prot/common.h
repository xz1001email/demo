
#ifndef  __SB_COMMON_H__
#define  __SB_COMMON_H__

#include "prot.h"
#include <queue>
#include <stdint.h>
using namespace std;


/**********queue and repeat_send struct****************/
#define PTR_QUEUE_BUF_CNT    (16)
#define UCHAR_QUEUE_SIZE    (128*1024)

#define MSG_ACK_READY           0
#define MSG_ACK_WAITING         1
#define MSG_NO_NEED_ACK         2

#define NO_MESSAGE_TO_SEND      1
#define NO_MESSAGE_TO_SEND      1

typedef struct queue_node_status{
    uint8_t cmd;
    uint8_t index;
    uint8_t ack_status;
    uint8_t send_repeat;
    struct timeval send_time;
    MmPacketIndex mm;
    bool mm_data_trans_waiting;
}SendStatus;

typedef struct _ptr_queue_node{
    uint8_t cmd;
    
    SBMmHeader2 mm_info;
    SendStatus pkg;
    InfoForStore mm;

    uint32_t len;
    uint8_t *buf;
} __attribute__((packed)) ptr_queue_node;

typedef struct _package_repeat_status{
#define REPEAT_SEND_TIMES_MAX   3
    char filepath[100];
    MmPacketIndex mm;

    bool mm_data_trans_waiting;
    uint8_t repeat_cnt;
    struct timeval msg_sendtime;
    bool start_wait_ack;
    ptr_queue_node msgsend;
} __attribute__((packed)) pkg_repeat_status;


//queue
int ptr_queue_push(queue<ptr_queue_node *> *p, ptr_queue_node *in,  pthread_mutex_t *lock);
int ptr_queue_pop(queue<ptr_queue_node*> *p, ptr_queue_node *out,  pthread_mutex_t *lock);
void push_mm_queue(InfoForStore *mm);
int pull_mm_queue(InfoForStore *mm);
void push_mm_req_cmd_queue(SBMmHeader2 *mm_info);
int pull_mm_req_cmd_queue(SBMmHeader2 *mm_info);

//list
void display_mm_resource();
int32_t find_mm_resource(uint32_t id, MmInfo_node *m);
int32_t delete_mm_resource(uint32_t id);
void insert_mm_resouce(MmInfo_node m);


int pull_mm_queue(InfoForStore *mm);
void push_mm_queue(InfoForStore *mm);
void display_mm_resource();
int32_t find_mm_resource(uint32_t id, MmInfo_node *m);
int32_t delete_mm_resource(uint32_t id);
void insert_mm_resouce(MmInfo_node m);

//common
#define DEBUG_G
#ifdef DEBUG_G 
//#define MY_DEBUG(format,...) printf("File: "__FILE__", Line: %05d:\n", __LINE__, ##__VA_ARGS__)
#define WSI_DEBUG(format,...) printf(format, ##__VA_ARGS__)
#else
#define WSI_DEBUG(format,...)
#endif
void printbuf(void *buf, int len);
int timeout_trigger(struct timespec *tv, int sec);

void *data_log_init(const char *filename, bool truncate);
void data_log(const char *log);

#endif
