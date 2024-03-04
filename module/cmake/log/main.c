#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "log_level.h"

unsigned char g_log_level = 0x0f;
extern int minieye_log(void);

void printbuf(void *buffer, int len)
{
    int i;
    uint8_t *buf = (uint8_t *)buffer;

    for (i=0; i<len; i++)
    {
        if (i && (i%16==0))
            LOGD("\n");

        if (i && (i%8==0) && (i%16!=0))
            LOGD("_0x%02x", buf[i]);
        else
            LOGD(" 0x%02x", buf[i]);
    }
    LOGD("\n");
}


int main()
{
    int i;
    int err = 5;
    int warn = 4;
    int info = 3;
    int debug = 2;
    uint8_t array[128];

    LOGE("err = %d\n", err); 
    LOGW("warn = %d\n", warn); 
    LOGI("info = %d\n", info); 
    LOGD("debug = %d\n", debug); 
    
    minieye_log();

    for (i=0; i<sizeof(array); i++) {
        array[i] = rand()%0xFF;
    }

    printbuf(array, sizeof(array));

    return 0;
}

