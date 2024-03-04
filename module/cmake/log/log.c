#include <stdio.h>
#include <stdint.h>
#include "log_level.h"


int minieye_log(void)
{
    int err = 5;
    int warn = 4;
    int info = 3;
    int debug = 2;
#if 1
    LOGE("err = %d\n", err); 
    LOGW("warn = %d\n", warn); 
    LOGI("info = %d\n", info); 
    LOGD("debug = %d\n", debug); 
#endif
    return 0;
}
