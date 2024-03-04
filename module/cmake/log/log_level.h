
#ifndef __COMMON_DEBUG__
#define __COMMON_DEBUG__

#include <stdio.h>

/* printf level */

#define LEVEL_ERR        0x01
#define LEVEL_WARNING    0x02
#define LEVEL_INFO       0x04
#define LEVEL_DEBUG      0x08

extern unsigned char g_log_level;
//uint8_t g_log_level;

#define dprintf(format,...) printf(format, ##__VA_ARGS__)

#define LOGE(format,...) do{if(g_log_level & LEVEL_ERR)printf(format, ##__VA_ARGS__);}while(0)
#define LOGW(format,...) do{if(g_log_level & LEVEL_WARNING)printf(format, ##__VA_ARGS__);}while(0)
#define LOGI(format,...) do{if(g_log_level & LEVEL_INFO)printf(format, ##__VA_ARGS__);}while(0)
#define LOGD(format,...) do{if(g_log_level & LEVEL_DEBUG)printf(format, ##__VA_ARGS__);}while(0)



#endif
