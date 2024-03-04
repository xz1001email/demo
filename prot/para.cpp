#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdint.h>
#include <getopt.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include <string.h>
#include <sys/time.h>

#include "prot.h"
#include "common.h"


static AdasParaSetting adas_para;
static DmsParaSetting dms_para;

static pthread_mutex_t para_lock = PTHREAD_MUTEX_INITIALIZER;
void read_dev_para(void *para, uint8_t para_type)
{
    //DmsParaSetting *dms_para = (DmsParaSetting *)para;
    //AdasParaSetting *adas_para = (AdasParaSetting *)para;

    pthread_mutex_lock(&para_lock);

    if(para_type == SAMPLE_DEVICE_ID_ADAS)
        memcpy(para, &adas_para, sizeof(adas_para));
    else if(para_type == SAMPLE_DEVICE_ID_DMS)
        memcpy(para, &dms_para, sizeof(dms_para));

    pthread_mutex_unlock(&para_lock);
}
void write_dev_para(void *para, uint8_t para_type)
{
    uint32_t i;
    uint8_t *in8 = NULL;
    uint8_t *out8 = NULL;
    DmsParaSetting *pDsmPara = (DmsParaSetting *)para;
    AdasParaSetting *pAdasPara = (AdasParaSetting *)para;

    in8 = (uint8_t *)para;

    pthread_mutex_lock(&para_lock);
    if(para_type == SAMPLE_DEVICE_ID_ADAS){
        printf("writing adas para...\n");
        out8 = (uint8_t *)&adas_para;
        if(pAdasPara->auto_photo_time_period != 0xFFFF)
            adas_para.auto_photo_time_period = pAdasPara->auto_photo_time_period;
        if(pAdasPara->auto_photo_distance_period != 0xFFFF)
            adas_para.auto_photo_distance_period = pAdasPara->auto_photo_distance_period;

        for(i=0; i< sizeof(AdasParaSetting); i++)
        {
            if(i==3)
                i+=4;

            if(in8[i] != 0xFF)
            {
                out8[i] = in8[i];
            }
        }

        adas_para_check(&adas_para);
    }else if(para_type == SAMPLE_DEVICE_ID_DMS){
        printf("writing dms para...\n");
        out8 = (uint8_t *)&dms_para;

        if(pDsmPara->auto_photo_time_period != 0xFFFF)
            dms_para.auto_photo_time_period = pDsmPara->auto_photo_time_period;
        if(pDsmPara->auto_photo_distance_period != 0xFFFF)
            dms_para.auto_photo_distance_period = pDsmPara->auto_photo_distance_period;

        if(pDsmPara->Smoke_TimeIntervalThreshold != 0xFFFF)
            dms_para.Smoke_TimeIntervalThreshold= pDsmPara->Smoke_TimeIntervalThreshold;

        if(pDsmPara->Call_TimeIntervalThreshold!= 0xFFFF)
            dms_para.Call_TimeIntervalThreshold= pDsmPara->Call_TimeIntervalThreshold;

        for(i=0; i< sizeof(DmsParaSetting); i++)
        {
            if(i==3)
                i+=4;
            if(i==21)
                i+=4;

            if(in8[i] != 0xFF)
            {
                out8[i] = in8[i];
            }
        }
    }
    pthread_mutex_unlock(&para_lock);
}

void print_adas_para(AdasParaSetting *para)
{
    printf("print adas para.................\n");

    printf("adas_para->warning_speed_val       = %d\n", para->warning_speed_val);
    printf("adas_para->warning_volume          = %d\n", para->warning_volume);
    printf("adas_para->auto_photo_mode         = %d\n", para->auto_photo_mode);
    printf("adas_para->auto_photo_time_period  = %d\n", (para->auto_photo_time_period));
    printf("adas_para->auto_photo_distance_peri= %d\n", (para->auto_photo_distance_period));
    printf("adas_para->photo_num               = %d\n", para->photo_num);
    printf("adas_para->photo_time_period       = %d\n", para->photo_time_period);
    printf("adas_para->image_Resolution        = %d\n", para->image_Resolution);
    printf("adas_para->video_Resolution        = %d\n", para->video_Resolution);
    printf("adas_para->obstacle_distance_thresh= %d\n", para->obstacle_distance_threshold);
    printf("adas_para->obstacle_video_time     = %d\n", para->obstacle_video_time);
    printf("adas_para->obstacle_photo_num      = %d\n", para->obstacle_photo_num);
    printf("adas_para->obstacle_photo_time_peri= %d\n", para->obstacle_photo_time_period);
    printf("adas_para->FLC_time_threshold      = %d\n", para->FLC_time_threshold);
    printf("adas_para->FLC_times_threshold     = %d\n", para->FLC_times_threshold);
    printf("adas_para->FLC_video_time          = %d\n", para->FLC_video_time);
    printf("adas_para->FLC_photo_num           = %d\n", para->FLC_photo_num);
    printf("adas_para->FLC_photo_time_period   = %d\n", para->FLC_photo_time_period);
    printf("adas_para->LDW_video_time          = %d\n", para->LDW_video_time);
    printf("adas_para->LDW_photo_num           = %d\n", para->LDW_photo_num);
    printf("adas_para->LDW_photo_time_period   = %d\n", para->LDW_photo_time_period);
    printf("adas_para->FCW_time_threshold      = %d\n", para->FCW_time_threshold);
    printf("adas_para->FCW_video_time          = %d\n", para->FCW_video_time);
    printf("adas_para->FCW_photo_num           = %d\n", para->FCW_photo_num);
    printf("adas_para->FCW_photo_time_period   = %d\n", para->FCW_photo_time_period);
    printf("adas_para->PCW_time_threshold      = %d\n", para->PCW_time_threshold);
    printf("adas_para->PCW_video_time          = %d\n", para->PCW_video_time);
    printf("adas_para->PCW_photo_num           = %d\n", para->PCW_photo_num);
    printf("adas_para->PCW_photo_time_period   = %d\n", para->PCW_photo_time_period);
    printf("adas_para->HW_time_threshold       = %d\n", para->HW_time_threshold);
    printf("adas_para->HW_video_time           = %d\n", para->HW_video_time);
    printf("adas_para->HW_photo_num            = %d\n", para->HW_photo_num);
    printf("adas_para->HW_photo_time_period    = %d\n", para->HW_photo_time_period);
    printf("adas_para->TSR_photo_num           = %d\n", para->TSR_photo_num);
    printf("adas_para->TSR_photo_time_period   = %d\n", para->TSR_photo_time_period);
}


void print_dms_para(DmsParaSetting *para)
{
    printf("print dms para.................\n");
    printf("dms_para->warning_speed_val             = %d\n", para->warning_speed_val);
    printf("dms_para->warning_volume                = %d\n", para->warning_volume);
    printf("dms_para->auto_photo_mode               = %d\n", para->auto_photo_mode);
    printf("dms_para->auto_photo_time_period        = %d\n", para->auto_photo_time_period);
    printf("dms_para->auto_photo_distance_period    = %d\n", para->auto_photo_distance_period);
    printf("dms_para->photo_num                     = %d\n", para->photo_num);
    printf("dms_para->photo_time_period             = %d\n", para->photo_time_period);
    printf("dms_para->image_Resolution              = %d\n", para->image_Resolution);
    printf("dms_para->video_Resolution              = %d\n", para->video_Resolution);
    printf("dms_para->Smoke_TimeIntervalThreshold   = %d\n", para->Smoke_TimeIntervalThreshold);
    printf("dms_para->Call_TimeIntervalThreshold    = %d\n", para->Call_TimeIntervalThreshold);
    printf("dms_para->FatigueDriv_VideoTime         = %d\n", para->FatigueDriv_VideoTime);
    printf("dms_para->FatigueDriv_PhotoNum          = %d\n", para->FatigueDriv_PhotoNum);
    printf("dms_para->FatigueDriv_PhotoInterval     = %d\n", para->FatigueDriv_PhotoInterval);
    printf("dms_para->FatigueDriv_resv              = %d\n", para->FatigueDriv_resv);
    printf("dms_para->CallingDriv_VideoTime         = %d\n", para->CallingDriv_VideoTime);
    printf("dms_para->CallingDriv_PhotoNum          = %d\n", para->CallingDriv_PhotoNum);
    printf("dms_para->CallingDriv_PhotoInterval     = %d\n", para->CallingDriv_PhotoInterval);
    printf("dms_para->SmokingDriv_VideoTime         = %d\n", para->SmokingDriv_VideoTime);
    printf("dms_para->SmokingDriv_PhotoNum          = %d\n", para->SmokingDriv_PhotoNum);
    printf("dms_para->SmokingDriv_PhotoInterval     = %d\n", para->SmokingDriv_PhotoInterval);

    printf("dms_para->DistractionDriv_VideoTime        = %d\n", para->DistractionDriv_VideoTime);
    printf("dms_para->DistractionDriv_PhotoNum         = %d\n", para->DistractionDriv_PhotoNum);
    printf("dms_para->DistractionDriv_PhotoInterval    = %d\n", para->DistractionDriv_PhotoInterval);
    printf("dms_para->AbnormalDriv_VideoTime     = %d\n", para->AbnormalDriv_VideoTime);
    printf("dms_para->AbnormalDriv_PhotoNum      = %d\n", para->AbnormalDriv_PhotoNum);
    printf("dms_para->AbnormalDriv_PhotoInterval = %d\n", para->AbnormalDriv_PhotoInterval);
}


void dms_para_check(DmsParaSetting *p)
{
    DmsParaSetting *para = (DmsParaSetting *)p;

    printf("dms para checking...!\n");
    if(para->warning_speed_val < 0 || para->warning_speed_val > 60)
    {
        para->warning_speed_val = 30;// km/h, 0-60
        printf("warn speed invalid, set to default!\n");
    }
    if(para->warning_volume < 0 || para->warning_volume > 8)
    {
        para->warning_volume = 6;//0~8
        printf("warn volume invalid, set to default!\n");
    }

    //initiative
    if(para->auto_photo_mode < 0 || para->auto_photo_mode > 3)
    {
        para->auto_photo_mode = 0;//主动拍照默认关闭
        printf("warn photo para invalid, set to default!\n");
    }
    if(para->auto_photo_time_period <0 || para->auto_photo_time_period > 3600)
    {
        printf("warn para invalid, set to default!\n");
        para->auto_photo_time_period = 1800; //单位：秒, 0~3600
    }
    if(para->auto_photo_distance_period < 0 || para->auto_photo_distance_period > 60000)
    {
        printf("warn para invalid, set to default!\n");
        para->auto_photo_distance_period = 100; //单位：米, 0-60000
    }

    //photo
    if(para->photo_num < 1 || para->photo_num > 10)
    {
        printf("warn photo invalid, set to default!\n");
        para->photo_num = 3;//1-10
    }
    if(para->photo_time_period < 1 || para->photo_time_period > 5)
    {
        printf("warn photo invalid, set to default!\n");
        para->photo_time_period = 2; //单位：100ms, 1-5
    }
    if(para->image_Resolution < 1 || para->image_Resolution > 6)
    {
        printf("warn photo invalid, set to default!\n");
        para->image_Resolution = 1;
    }
    if(para->video_Resolution < 1 || para->video_Resolution > 7)
    {
        printf("warn photo invalid, set to default!\n");
        para->video_Resolution = 1;
    }

    //smoke
    if(para->Smoke_TimeIntervalThreshold < 0 || para->Smoke_TimeIntervalThreshold > 3600)
    {
        printf("smoke invalid, set to default!\n");
        para->Smoke_TimeIntervalThreshold= 180; // 单位：100ms
    }
    if(para->Call_TimeIntervalThreshold < 0 || para->Call_TimeIntervalThreshold > 3600)
    {
        printf("call invalid, set to default!\n");
        para->Call_TimeIntervalThreshold = 120; //单位：秒
    }
    //fatigue
    if(para->FatigueDriv_VideoTime < 0 || para->FatigueDriv_VideoTime > 60)
    {
        printf("warn FatigueDriv invalid, set to default!\n");
        para->FatigueDriv_VideoTime = 5;
    }
    if(para->FatigueDriv_PhotoNum < 0 || para->FatigueDriv_PhotoNum > 10)
    {
        printf("warn FatigueDriv invalid, set to default!\n");
        para->FatigueDriv_PhotoNum = 3; // 单位：100ms
    }
    if(para->FatigueDriv_PhotoInterval < 1 || para->FatigueDriv_PhotoInterval > 5)
    {
        printf("warn FatigueDriv invalid, set to default!\n");
        para->FatigueDriv_PhotoInterval = 2; // 单位：100ms
    }
    //call
    if(para->CallingDriv_VideoTime < 0 || para->CallingDriv_VideoTime > 60)
    {
        printf("warn CallingDriv invalid, set to default!\n");
        para->CallingDriv_VideoTime = 5;
    }
    if(para->CallingDriv_PhotoNum < 0 || para->CallingDriv_PhotoNum > 10)
    {
        printf("warn CallingDriv invalid, set to default!\n");
        para->CallingDriv_PhotoNum = 3; // 单位：100ms
    }
    if(para->CallingDriv_PhotoInterval < 1 || para->CallingDriv_PhotoInterval > 5)
    {
        printf("warn CallingDriv invalid, set to default!\n");
        para->CallingDriv_PhotoInterval = 2; // 单位：100ms
    }
    //smoke
    if(para->SmokingDriv_VideoTime < 0 || para->SmokingDriv_VideoTime > 60)
    {
        printf("warn SmokingDriv invalid, set to default!\n");
        para->SmokingDriv_VideoTime = 5;
    }
    if(para->SmokingDriv_PhotoNum < 0 || para->SmokingDriv_PhotoNum > 10)
    {
        printf("warn SmokingDriv invalid, set to default!\n");
        para->SmokingDriv_PhotoNum = 3; // 单位：100ms
    }
    if(para->SmokingDriv_PhotoInterval < 1 || para->SmokingDriv_PhotoInterval > 5)
    {
        printf("warn SmokingDriv invalid, set to default!\n");
        para->SmokingDriv_PhotoInterval = 2; // 单位：100ms
    }
    //distraction
    if(para->DistractionDriv_VideoTime < 0 || para->DistractionDriv_VideoTime > 60)
    {
        printf("warn DistractionDriv invalid, set to default!\n");
        para->DistractionDriv_VideoTime = 5;
    }
    if(para->DistractionDriv_PhotoNum < 0 || para->DistractionDriv_PhotoNum > 10)
    {
        printf("warn DistractionDriv invalid, set to default!\n");
        para->DistractionDriv_PhotoNum = 3; // 单位：100ms
    }
    if(para->DistractionDriv_PhotoInterval < 1 || para->DistractionDriv_PhotoInterval > 5)
    {
        printf("warn DistractionDriv invalid, set to default!\n");
        para->DistractionDriv_PhotoInterval = 2; // 单位：100ms
    }
    //abnormal
    if(para->AbnormalDriv_VideoTime < 0 || para->AbnormalDriv_VideoTime > 60)
    {
        printf("warn AbnormalDriv invalid, set to default!\n");
        para->AbnormalDriv_VideoTime = 5;
    }
    if(para->AbnormalDriv_PhotoNum < 0 || para->AbnormalDriv_PhotoNum > 10)
    {
        printf("warn AbnormalDriv invalid, set to default!\n");
        para->AbnormalDriv_PhotoNum = 3; // 单位：100ms
    }
    if(para->AbnormalDriv_PhotoInterval < 1 || para->AbnormalDriv_PhotoInterval > 5)
    {
        printf("warn AbnormalDriv invalid, set to default!\n");
        para->AbnormalDriv_PhotoInterval = 2; // 单位：100ms
    }

}

//if para error, set default val
void adas_para_check(AdasParaSetting *p)
{
    AdasParaSetting *para = (AdasParaSetting *)p;

    printf("adas para checking...!\n");
    
    if(para->warning_speed_val < 0 || para->warning_speed_val > 60)
    {
        para->warning_speed_val = 30;// km/h, 0-60
        printf("warn speed invalid, set to default!\n");
    }
    if(para->warning_volume < 0 || para->warning_volume > 8)
    {
        para->warning_volume = 6;//0~8
        printf("warn volume invalid, set to default!\n");
    }

    //initiative
    if(para->auto_photo_mode < 0 || para->auto_photo_mode > 3)
    {
        para->auto_photo_mode = 0;//主动拍照默认关闭
        printf("warn photo para invalid, set to default!\n");
    }
    if(para->auto_photo_time_period <0 || para->auto_photo_time_period > 3600)
    {
        printf("warn para invalid, set to default!\n");
        para->auto_photo_time_period = 1800; //单位：秒, 0~3600
    }
    if(para->auto_photo_distance_period < 0 || para->auto_photo_distance_period > 60000)
    {
        printf("warn para invalid, set to default!\n");
        para->auto_photo_distance_period = 100; //单位：米, 0-60000
    }

    //photo
    if(para->photo_num < 1 || para->photo_num > 10)
    {
        printf("warn photo invalid, set to default!\n");
        para->photo_num = 3;//1-10
    }
    if(para->photo_time_period < 1 || para->photo_time_period > 5)
    {
        printf("warn photo invalid, set to default!\n");
        para->photo_time_period = 2; //单位：100ms, 1-5
    }
    if(para->image_Resolution < 1 || para->image_Resolution > 6)
    {
        printf("warn photo invalid, set to default!\n");
        para->image_Resolution = 1;
    }
    if(para->video_Resolution < 1 || para->video_Resolution > 7)
    {
        printf("warn photo invalid, set to default!\n");
        para->video_Resolution = 1;
    }

    //obstacle
    if(para->obstacle_distance_threshold < 10 || para->obstacle_distance_threshold > 50)
    {
        printf("warn obstacle invalid, set to default!\n");
        para->obstacle_distance_threshold = 30; // 单位：100ms
    }
    if(para->obstacle_video_time < 0 || para->obstacle_video_time > 60)
    {
        printf("warn obstacle invalid, set to default!\n");
        para->obstacle_video_time = 5; //单位：秒
    }
    if(para->obstacle_photo_num < 0 || para->obstacle_photo_num > 10)
    {
        printf("warn obstacle invalid, set to default!\n");
        para->obstacle_photo_num = 3;
    }
    if(para->obstacle_photo_time_period < 1 || para->obstacle_photo_time_period > 10)
    {
        printf("warn obstacle invalid, set to default!\n");
        para->obstacle_photo_time_period = 2; // 单位：100ms
    }

    //FLC
    if(para->FLC_time_threshold < 30 || para->FLC_time_threshold >120)
    {
        printf("warn FLC invalid, set to default!\n");
        para->FLC_time_threshold = 60;
    }
    if(para->FLC_times_threshold < 3 || para->FLC_times_threshold >10)
    {
        printf("warn FLC invalid, set to default!\n");
        para->FLC_times_threshold = 5 ;
    }
    if(para->FLC_video_time < 0 || para->FLC_video_time > 60)
    {
        printf("warn FLC invalid, set to default!\n");
        para->FLC_video_time = 5;
    }
    if(para->FLC_photo_num < 0 || para->FLC_photo_num >10)
    {
        printf("warn FLC invalid, set to default!\n");
        para->FLC_photo_num = 3;
    }
    if(para->FLC_photo_time_period < 1 || para->FLC_photo_time_period > 10)
    {
        printf("warn FLC invalid, set to default!\n");
        para->FLC_photo_time_period = 2;
    }

    //LDW
    if(para->LDW_video_time < 0 || para->LDW_video_time > 60)
    {
        printf("warn LDW invalid, set to default!\n");
        para->LDW_video_time = 5;
    }
    if(para->LDW_photo_num < 0 || para->LDW_photo_num > 10)
    {
        printf("warn LDW invalid, set to default!\n");
        para->LDW_photo_num = 3;
    }
    if(para->LDW_photo_time_period < 1 || para->LDW_photo_time_period > 10)
    {
        printf("warn LDW invalid, set to default!\n");
        para->LDW_photo_time_period = 2;
    }

    //FCW
    if(para->FCW_time_threshold < 10 || para->FCW_time_threshold > 50)
    {
        printf("warn FCW invalid, set to default!\n");
        para->FCW_time_threshold = 27;
    }
    if(para->FCW_video_time < 0 || para->FCW_video_time > 60)
    {
        printf("warn FCW invalid, set to default!\n");
        para->FCW_video_time = 5;
    }
    if(para->FCW_photo_num < 0 || para->FCW_photo_num > 10)
    {
        printf("warn FCW invalid, set to default!\n");
        para->FCW_photo_num = 3;
    }
    if(para->FCW_photo_time_period < 1 || para->FCW_photo_time_period > 10)
    {
        printf("warn FCW invalid, set to default!\n");
        para->FCW_photo_time_period = 2;
    }

    //PCW
    if(para->PCW_time_threshold < 0 || para->PCW_time_threshold > 50)
    {
        printf("PCW threshold invalid, set to default!\n");
        para->PCW_time_threshold = 30;
    }
    if(para->PCW_video_time < 0 || para->PCW_video_time > 60)
    {
        printf("pcw video time invalid, set to default!\n");
        para->PCW_video_time = 5;
    }
    if(para->PCW_photo_num < 0 || para->PCW_photo_num > 10)
    {
        printf("PCW photo num invalid, set to default!\n");
        para->PCW_photo_num = 3;
    }
    if(para->PCW_photo_time_period < 1 || para->PCW_photo_time_period > 10)
    {
        printf("PCW time period invalid, set to default!\n");
        para->PCW_photo_time_period = 2;
    }

    //HMW
    if(para->HW_time_threshold < 0 ||  para->HW_time_threshold > 50)
    {
        printf("warn HMW invalid, set to default!\n");
        para->HW_time_threshold = 30; // 单位：100ms 
    }
    if(para->HW_video_time < 0 || para->HW_video_time > 60)
    {
        printf("warn HMW invalid, set to default!\n");
        para->HW_video_time = 5;
    }
    if(para->HW_photo_num < 0 || para->HW_photo_num > 10)
    {
        printf("warn HMW invalid, set to default!\n");
        para->HW_photo_num = 3;
    }
    if(para->HW_photo_time_period < 1 || para->HW_photo_time_period > 10)
    {
        printf("warn HMW invalid, set to default!\n");
        para->HW_photo_time_period = 2; // 单位：100ms
    }

    //TSR
    if(para->TSR_photo_num < 0 || para->TSR_photo_num > 10)
    {
        printf("warn TSR invalid, set to default!\n");
        para->TSR_photo_num = 3;
    }
    if(para->TSR_photo_time_period < 1 || para->TSR_photo_time_period > 10)
    {
        printf("warn TSR invalid, set to default!\n");
        para->TSR_photo_time_period = 2;
    }
}

void set_AdasParaSetting_default()
{
    AdasParaSetting para;
    
    printf("%s!\n", __FUNCTION__);
    para.warning_speed_val = 30;// km/h, 0-60
    para.warning_volume = 6;//0~8
    para.auto_photo_mode = 0;//主动拍照默认关闭
    para.auto_photo_time_period = 1800; //单位：秒, 0~3600
    para.auto_photo_distance_period = 100; //单位：米, 0-60000
    para.photo_num = 3;//1-10
    para.photo_time_period = 2; //单位：100ms, 1-5
    para.image_Resolution = 1;
    para.video_Resolution = 1;
    memset(&para.reserve[0], 0, sizeof(para.reserve));

    para.obstacle_distance_threshold = 30; // 单位：100ms
    para.obstacle_video_time = 5; //单位：秒
    para.obstacle_photo_num = 3;
    para.obstacle_photo_time_period = 2; // 单位：100ms

    para.FLC_time_threshold = 60;
    para.FLC_times_threshold = 5 ;
    para.FLC_video_time = 5;
    para.FLC_photo_num = 3;
    para.FLC_photo_time_period = 2;

    para.LDW_video_time = 5;
    para.LDW_photo_num = 3;
    para.LDW_photo_time_period = 2;

    para.FCW_time_threshold = 27;
    para.FCW_video_time = 5;
    para.FCW_photo_num = 3;
    para.FCW_photo_time_period = 2;

    para.PCW_time_threshold = 30;
    para.PCW_video_time = 5;
    para.PCW_photo_num = 3;
    para.PCW_photo_time_period = 2;

    para.HW_time_threshold = 30; // 单位：100ms 
    para.HW_video_time = 5;
    para.HW_photo_num = 3;
    para.HW_photo_time_period = 2; // 单位：100ms

    para.TSR_photo_num = 3;
    para.TSR_photo_time_period = 2;

    memset(&para.reserve2[0], 0, sizeof(para.reserve2));

    write_dev_para(&para, SAMPLE_DEVICE_ID_ADAS);

    printf("write default adas para to global para!\n");
}

void set_DmsParaSetting_default()
{
    DmsParaSetting para;

    printf("%s!\n", __FUNCTION__);
    para.warning_speed_val = 30;// km/h, 0-60
    para.warning_volume = 6;//0~8
    para.auto_photo_mode = 0;//主动拍照默认关闭
    para.auto_photo_time_period = 1800; //单位：秒, 0~3600
    para.auto_photo_distance_period = 100; //单位：米, 0-60000
    para.photo_num = 3;//1-10
    para.photo_time_period = 2; //单位：100ms, 1-5
    para.image_Resolution = 1;
    para.video_Resolution = 1;
    memset(&para.reserve[0], 0, sizeof(para.reserve));

    para.Smoke_TimeIntervalThreshold = 180;
    para.Call_TimeIntervalThreshold = 120;

    para.FatigueDriv_VideoTime = 5;
    para.FatigueDriv_PhotoNum = 3;
    para.FatigueDriv_PhotoInterval = 2; //100ms
    para.FatigueDriv_resv = 0;

    para.CallingDriv_VideoTime = 5;
    para.CallingDriv_PhotoNum = 3;
    para.CallingDriv_PhotoInterval = 2;

    para.SmokingDriv_VideoTime = 5;
    para.SmokingDriv_PhotoNum = 3;
    para.SmokingDriv_PhotoInterval = 2;

    para.DistractionDriv_VideoTime = 5;
    para.DistractionDriv_PhotoNum = 3;
    para.DistractionDriv_PhotoInterval = 2;

    para.AbnormalDriv_VideoTime = 5;
    para.AbnormalDriv_PhotoNum = 3;
    para.AbnormalDriv_PhotoInterval = 2;

    memset(&para.reserve2[0], 0, sizeof(para.reserve2));

    write_dev_para(&para, SAMPLE_DEVICE_ID_DMS);
    printf("write default dms para to global para!\n");
}

int read_local_adas_para_file(const char* filename)
{
    AdasParaSetting para;
    char cmd[512];

    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open for write: %s, so create\n", filename);
        fp = fopen(filename, "w+");
        if (!fp) {
            fprintf(stderr, "create: %s, file\n", filename);
            return 1;
        }
        fclose(fp);

        //no para file ,so set default to file
        set_AdasParaSetting_default();
        write_local_adas_para_file(filename);
    }else{
        size_t nb = fread(&para, 1, sizeof(para), fp);
        if (nb != sizeof(para)) {
            fprintf(stderr, "Error: didn't read enough bytes (%zd/%zd)\n", nb, sizeof(para));
            fclose(fp);
            return 2;
        }
        printf("write local file to global adas para\n");
        write_dev_para(&para, SAMPLE_DEVICE_ID_ADAS);
        print_adas_para(&para);

#if 0
        //set alog detect.flag
        sprintf(cmd, "busybox sed -i 's/^.*--test_speed.*$/--test_speed=%d/' /data/xiao/install/detect.flag",\
                para.warning_speed_val);
#endif

#if 0
        //set alog detect.flag
        sprintf(cmd, "busybox sed -i 's/^.*--output_lane_info_speed_thresh.*$/--output_lane_info_speed_thresh=%d/' /data/xiao/install/detect.flag",\
                para.warning_speed_val);


        system(cmd);

#endif
        fclose(fp);
    }
    return 0;
}


int read_local_dms_para_file(const char* filename)
{
    DmsParaSetting para;
    char cmd[512];

    FILE* fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open for write: %s, so create\n", filename);
        fp = fopen(filename, "w+");
        if (!fp) {
            fprintf(stderr, "create: %s, file\n", filename);
            return 1;
        }
        fclose(fp);

        //no para file ,so set default to file
        set_DmsParaSetting_default();
        write_local_dms_para_file(filename);
    }else{
        size_t nb = fread(&para, 1, sizeof(para), fp);
        if (nb != sizeof(para)) {
            fprintf(stderr, "Error: didn't read enough bytes (%zd/%zd)\n", nb, sizeof(para));
            fclose(fp);
            return 2;
        }
        printf("write local file to global dms para\n");
        write_dev_para(&para, SAMPLE_DEVICE_ID_DMS);
        print_dms_para(&para);

#if 0
        //set alog detect.flag
        sprintf(cmd, "busybox sed -i 's/^.*--test_speed.*$/--test_speed=%d/' /data/xiao/install/detect.flag",\
                para.warning_speed_val);
#endif

#if 0
        //set alog detect.flag
        sprintf(cmd, "busybox sed -i 's/^.*--output_lane_info_speed_thresh.*$/--output_lane_info_speed_thresh=%d/' /data/xiao/install/detect.flag",\
                para.warning_speed_val);


        system(cmd);

#endif
        fclose(fp);
    }
    return 0;
}


int write_local_adas_para_file(const char* filename) {
    int ret = 0;
    AdasParaSetting para;

    read_dev_para(&para, SAMPLE_DEVICE_ID_ADAS);
    print_adas_para(&para);
    ret = write_file(filename, &para, sizeof(para));

    return ret;
}
int write_local_dms_para_file(const char* filename) {
    int ret = 0;
    DmsParaSetting dms_para;

    read_dev_para(&dms_para, SAMPLE_DEVICE_ID_DMS);
    print_dms_para(&dms_para);
    ret = write_file(filename, &dms_para, sizeof(dms_para));

    return ret;
}


