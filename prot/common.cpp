#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdint.h>
#include <getopt.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <list>
#include <queue>

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <semaphore.h>
#include <sys/prctl.h>

#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


using namespace std;
#include "prot.h"
#include "common.h"


//return 0, 超时，单位是毫秒 
int timeout_trigger(struct timespec *tv, int sec)
{
    struct timespec cur;
    long last = 0, now = 0;

	clock_gettime(CLOCK_MONOTONIC, &cur);

    last =  tv->tv_sec*1000 + tv->tv_nsec/1000000;
    now =  cur.tv_sec*1000 + cur.tv_nsec/1000000;

    //if((cur.tv_sec >= tv->tv_sec + sec) && (cur.tv_nsec > tv->tv_nsec)){
    if(now - last > sec*1000){
        //printf("timeout_trigger! %d s\n", sec);
        return 1;
    }
    else
        return 0;
}


//推入队列，可以只有node的header，数据可以为空
int ptr_queue_push(queue<ptr_queue_node *> *p, ptr_queue_node *in,  pthread_mutex_t *lock)
{
    int ret;
    ptr_queue_node *header = NULL;
    uint8_t *ptr = NULL;

    if(!in || !p)
        return -1;

    pthread_mutex_lock(lock);
    if((int)p->size() == PTR_QUEUE_BUF_CNT)
    {
        printf("ptr queue overflow...\n");
        ret = -1;
        goto out;
    }
    else
    {
        header = (ptr_queue_node *)malloc(sizeof(ptr_queue_node));
        if(!header)
        {
            perror("ptr_queue_push malloc1");
            ret = -1;
            goto out;
        }

        memcpy(header, in, sizeof(ptr_queue_node));
        if(!in->buf)//user don't need buffer
        {
            header->len = 0;
            header->buf = NULL;
        }
        else //user need buffer
        {
            ptr = (uint8_t *)malloc(PTR_QUEUE_BUF_SIZE);
            if(!ptr)
            {
                perror("ptr_queue_push malloc2");
                free(header);
                ret = -1;
                goto out;
            }

            header->buf = ptr;
            header->len = in->len > PTR_QUEUE_BUF_SIZE ? PTR_QUEUE_BUF_SIZE : in->len;
            memcpy(header->buf, in->buf, header->len);
        }

        p->push(header);
        ret = 0;
        goto out;
    }

out:
    pthread_mutex_unlock(lock);
    return ret;
}

//弹出队列，可以只取node的header，数据不要
int ptr_queue_pop(queue<ptr_queue_node*> *p, ptr_queue_node *out,  pthread_mutex_t *lock)
{
    ptr_queue_node *header = NULL;
    uint32_t user_buflen = 0;
    uint8_t *ptr = NULL;

    if(!out || !p)
        return -1;

    pthread_mutex_lock(lock);
    if(!p->empty())
    {
        header = p->front();
        p->pop();
    }
    pthread_mutex_unlock(lock);

    if(!header)
        return -1;

    //no data in node
    if(!header->buf)
    {
        //printf("no data in node!\n");
        memcpy(out, header, sizeof(ptr_queue_node));
    }
    //node have data,
    else
    {
        //user don't need data
        if(!out->buf)
        {
            //printf("user no need data out!\n");
            memcpy(out, header, sizeof(ptr_queue_node));
        }
        else
        {
            //record ptr and len
            ptr = out->buf;
            user_buflen = out->len;
            memcpy(out, header, sizeof(ptr_queue_node));
            out->buf = ptr;
            //get the min len
            header->len = header->len > PTR_QUEUE_BUF_SIZE ? PTR_QUEUE_BUF_SIZE : header->len;
            out->len = header->len > user_buflen ? user_buflen : header->len;
            memcpy(out->buf, header->buf, out->len);
        }
        free(header->buf);
    }

    free(header);
    return 0;
}




static list<MmInfo_node> mmlist;
static pthread_mutex_t mm_resource_lock = PTHREAD_MUTEX_INITIALIZER;

void display_mm_resource()
{
    list<MmInfo_node>::iterator it;  

    pthread_mutex_lock(&mm_resource_lock);
    for(it=mmlist.begin();it!=mmlist.end();it++)  
    {  
        printf("display list id = %d\n",it->mm_id);
        printf("warn_type = %d, mm_type=%d\n",it->warn_type, it->mm_type);
    }  
    pthread_mutex_unlock(&mm_resource_lock);
}

int32_t find_mm_resource(uint32_t id, MmInfo_node *m)
{
    list<MmInfo_node>::iterator it;  
    int ret = -1;

    pthread_mutex_lock(&mm_resource_lock);
    for(it=mmlist.begin();it!=mmlist.end();it++)  
    {  
        //  printf("find id=%d,list id = %d\n",id, it->mm_id);
        //  printf("warn_type = %d, mm_type=%d\n",it->warn_type, it->mm_type);
        if(it->mm_id == id)
        {
            memcpy(m, &it->rw_flag, sizeof(MmInfo_node));  
            ret = 0;
            break;
        }
    }  
    pthread_mutex_unlock(&mm_resource_lock);
    return ret;
}


void delete_file(uint32_t id)
{
    char filepath[100];

    mmid_to_filename(id, 0, filepath);
    printf("rm jpeg %s\n", filepath);
    remove(filepath);
}


int32_t delete_mm_resource(uint32_t id)
{
    list<MmInfo_node>::iterator it;  
    int ret = -1;

    pthread_mutex_lock(&mm_resource_lock);
    for(it=mmlist.begin();it!=mmlist.end();it++)  
    {  
        //   printf("delete id=%d, list id = %d\n",id, it->mm_id);
        //   printf("warn_type = %d, mm_type=%d\n",it->warn_type, it->mm_type);
        if(it->mm_id == id)
        {
            if(it->mm_type == MM_PHOTO)
            {
                //delete_file(id);
                it = mmlist.erase(it);  
                ret = 0;
                break;
            }
            if(it->mm_type == MM_VIDEO)
            {
                //delete_file(id);
                it = mmlist.erase(it);  
                ret = 0;
                break;
            }

        }
    }  
    pthread_mutex_unlock(&mm_resource_lock);

    return ret;
}

void insert_mm_resouce(MmInfo_node m)
{
    pthread_mutex_lock(&mm_resource_lock);
    mmlist.push_back(m); 
    pthread_mutex_unlock(&mm_resource_lock);
}


void printbuf(void *buffer, int len)
{
    int i;
    uint8_t *buf = (uint8_t *)buffer;

    for(i=0; i<len; i++)
    {
        if(i && (i%16==0))
            WSI_DEBUG("\n");

        WSI_DEBUG("0x%02x ", buf[i]);
    }
    WSI_DEBUG("\n");
}

size_t ReadFile(char *buf, int len, const char *filename)
{
    FILE *fp;
    size_t size = 0;

    fp = fopen(filename, "rb");
    if(!fp){
        printf("read file fail:%s\n", strerror(errno));
        return 0;
    }   
    size = fread(buf, 1, len, fp);
    fclose(fp);

    return size;
}



static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static FILE* log_fp = NULL;

static int create_logfile(const char *filename, bool truncate)
{
    if (!filename) {
        printf("filename is empty\n");
        exit(1);
    }

    log_fp = fopen(filename, truncate?"w":"a+");
    if (log_fp == NULL)
    {
        printf("open %s failed %s\n", filename, strerror(errno));
        exit(2);
        return -1;
    }
    return 0;
}

void *data_log_init(const char *filename, bool truncate)
{
    if (!log_fp)
        create_logfile(filename, truncate);
    return log_fp;
}

#if 0
#define LOG_TAG "my_data_log"
#include <utils/CallStack.h>
using namespace android;
extern "C" void dump_stack_android(void)
{
    CallStack stack;
    stack.update();
    stack.log(LOG_TAG);
}
#endif

void data_log(const char *log)
{
    static int line_count = 0;
    pthread_mutex_lock(&lock);
    //dump_stack_android();
    if (log_fp)
        fprintf(log_fp, "%s\n",log);
    else
        fprintf(stderr, "no log_fp\n");
    line_count ++;
    #define FLUSH_THRESHOLD (1)
    if (FLUSH_THRESHOLD == line_count) {
        fflush(log_fp);
        line_count = 0;
    }
    pthread_mutex_unlock(&lock);
}







