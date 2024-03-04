#ifndef  __SAMPLE_H__
#define __SAMPLE_H__
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

//#define ENABLE_ADAS
//#define ENABLE_DMS


#define HARDWARE_VERSION    "1.0.0"
#define SOFTWARE_VERSION    "1.2.0"


#define LOG_SIZE_1G (100*1024*1024)
//#define SAVE_MEDIA_NUM_MAX 100
#define SAVE_MEDIA_NUM_MAX 1000
#define SAVE_ANOTHER_CAMERA_VIDEO
//#define USE_CAN760_MESSAGE

/***********enable filter***********/
#define FILTER_ALERT_BY_SPEED
#define FILTER_ALERT_BY_TIME

/******set filter time 30 sec******/
#define FILTER_ADAS_ALERT_SET_TIME 10u
#define FILTER_DMS_ALERT_SET_TIME 10u


#define EXIT_MSG      2
#define NOTICE_MSG    1
#define WAIT_MSG      0
#define IS_EXIT_MSG(flag)   (flag == EXIT_MSG)

#if defined ENABLE_ADAS
#define SNAP_SHOT_JPEG_PATH "/data/snap/adas/"
#define SNAP_SHOT_PATH_FILES "/data/snap/adas/*"

#elif defined ENABLE_DMS
#define SNAP_SHOT_JPEG_PATH "/data/snap/dms/"
#define SNAP_SHOT_PATH_FILES "/data/snap/dms/*"
#endif

#define LOCAL_ADAS_PRAR_FILE     "/data/adas_para"
#define LOCAL_DMS_PRAR_FILE     "/data/dms_para"


#define UPGRADE_FILE_PATH     "/mnt/obb/prot_upgrade/"
#define UPGRADE_FILE_NAME     "/mnt/obb/prot_upgrade/subiao_upgrade.bin"
#define CLEAN_UPGRADE_BIN     "rm /mnt/obb/prot_upgrade/subiao_upgrade.bin"
#define UPGRADE_FILE_CMD      "/data/prot_upgrade.sh"

//protocol
#define PROTOCOL_USING_BIG_ENDIAN

#ifdef PROTOCOL_USING_BIG_ENDIAN
#define MY_HTONL(x)     htonl(x)
#define MY_HTONS(x)     htons(x)

#define MY_NTOHL(x)     ntohl(x)
#define MY_NTOHS(x)     ntohs(x)

#else
#define MY_HTONL(x)     (x)
#define MY_HTONS(x)     (x)
#define MY_NTOHL(x)     (x)
#define MY_NTOHS(x)     (x)

#endif


/*****************苏标 SB ********************/
#define SAMPLE_DEVICE_ID_BRDCST         (0x0)
#define SAMPLE_DEVICE_ID_ADAS           (0x64)
#define SAMPLE_DEVICE_ID_DMS            (0x65)
#define SAMPLE_DEVICE_ID_TPMS           (0x66)
#define SAMPLE_DEVICE_ID_BSD            (0x67)

#define SAMPLE_CMD_QUERY                (0x2F)
#define SAMPLE_CMD_FACTORY_RESET        (0x30)
#define SAMPLE_CMD_SPEED_INFO           (0x31)
#define SAMPLE_CMD_DEVICE_INFO          (0x32)
#define SAMPLE_CMD_UPGRADE              (0x33)
#define SAMPLE_CMD_GET_PARAM            (0x34)
#define SAMPLE_CMD_SET_PARAM            (0x35)
#define SAMPLE_CMD_WARNING_REPORT       (0x36)
#define SAMPLE_CMD_REQ_STATUS           (0x37)
#define SAMPLE_CMD_UPLOAD_STATUS        (0x38)
#define SAMPLE_CMD_REQ_MM_DATA          (0x50)
#define SAMPLE_CMD_UPLOAD_MM_DATA       (0x51)
#define SAMPLE_CMD_SNAP_SHOT            (0x52)

#define SAMPLE_PROT_MAGIC               (0x7E)
#define SAMPLE_PROT_ESC_CHAR            (0x7D)
#define MM_PHOTO 0
#define MM_AUDIO 1
#define MM_VIDEO 2

#define SANBAO_VERSION 0x01
#define VENDOR_ID 0x1234

#define SB_WARN_STATUS_BEGIN (0x1)
#define SB_WARN_STATUS_END   (0x2)
#define SB_WARN_STATUS_NONE (0x0)

#define WARN_TYPE_NUM       (8)
#define SB_WARN_TYPE_FCW     (0x1)
#define SB_WARN_TYPE_LDW     (0x2)
#define SB_WARN_TYPE_HW      (0x3)
#define SB_WARN_TYPE_PCW     (0x4)
#define SB_WARN_TYPE_FLC     (0x5)
#define SB_WARN_TYPE_TSRW    (0x6)

#define SB_WARN_TYPE_TSR     (0x10)
#define SB_WARN_TYPE_SNAP    (0x11)

#define SB_WARN_TSR_TYPE_SPEED   (0x1)
#define SB_WARN_TSR_TYPE_HIGHT   (0x2)
#define SB_WARN_TSR_TYPE_WEIGHT  (0x3)

typedef struct __SBProtHeader
{
    uint8_t     magic;
    uint8_t     checksum;
    uint16_t    serial_num;
    uint16_t    vendor_id;
    uint8_t     device_id;
    uint8_t     cmd;
    //uint8_t     payload[n];
    //uint8_t     magic1;
} __attribute__((packed)) SBProtHeader;

typedef struct __SBMmHeader
{
    uint8_t  type;
    uint32_t id;
} __attribute__((packed)) SBMmHeader;

typedef struct __SBMmHeader2
{
    uint8_t  devid;
    uint8_t  type;
    uint32_t id;
} __attribute__((packed)) SBMmHeader2;

typedef struct __M4DevInfo
{
    uint8_t     vendor_name_len;
    uint8_t     vendor_name[15];
    uint8_t     prod_code_len;
    uint8_t     prod_code[15];
    uint8_t     hw_ver_len;
    uint8_t     hw_ver[15];
    uint8_t     sw_ver_len;
    uint8_t     sw_ver[15];
    uint8_t     dev_id_len;
    uint8_t     dev_id[15];
    uint8_t     custom_code_len;
    uint8_t     custom_code[15];
} __attribute__((packed)) M4DevInfo;

typedef struct __MmPacketIndex
{
    uint8_t     type;
    uint32_t    id;
    uint16_t    packet_total_num;
    uint16_t    packet_index;
} __attribute__((packed)) MmPacketIndex;

typedef struct __MmAckInfo
{
    uint8_t     req_type;
    uint32_t    mm_id;
    uint16_t    packet_total_num;
    uint16_t    packet_index;
    uint8_t     ack;
} __attribute__((packed)) MmAckInfo;

typedef struct __CarStatus{
    uint16_t    acc:1;
    uint16_t    left_signal:1;
    uint16_t    right_signal:1;
    uint16_t    wipers:1;
    uint16_t    brakes:1;
    uint16_t    card:1;
    uint16_t    byte_resv:10;
} __attribute__((packed)) CarStatus;
typedef struct __RealTimeData{

    uint8_t     car_speed;
    uint8_t     reserve1;
    uint32_t    mileage;
    uint8_t     reserve2[2];

    uint16_t	altitude;
    uint32_t	latitude;
    uint32_t	longitude;

    uint8_t     time[6];
    CarStatus car_status;

} __attribute__((packed)) RealTimeData;


typedef struct __ModuleStatus{
    
#define MODULE_STANDBY          0x01
#define MODULE_WORKING          0x02
#define MODULE_MAINTAIN         0x03
#define MODULE_ABNORMAL         0x04
    uint8_t work_status;
    uint32_t camera_err:1;
    uint32_t main_memory_err:1;
    uint32_t aux_memory_err:1;
    uint32_t infrared_err:1;
    uint32_t speaker_err:1;
    uint32_t battery_err:1;
    uint32_t reserve6_err:1;
    uint32_t reserve7_err:1;
    uint32_t reserve8_err:1;
    uint32_t reserve9_err:1;
    uint32_t comm_module_err:1;
    uint32_t def_module_err:1;
    uint32_t reserve_err:20;
} __attribute__((packed)) ModuleStatus;

typedef struct __AdasWarnFrame {
    uint32_t	warning_id;
    uint8_t		status_flag;
    uint8_t		sound_type;
    uint8_t		forward_car_speed;
    uint8_t		forward_car_distance;
    uint8_t		ldw_type;
    uint8_t		load_type;
    uint8_t		load_data;
    uint8_t		car_speed;
    uint16_t	altitude;
    uint32_t	latitude;
    uint32_t	longitude;
    uint8_t		time[6];
    CarStatus	car_status;	
    uint8_t		mm_num;
    SBMmHeader mm[0];
} __attribute__((packed)) AdasWarnFrame;


// DMS warn Frame
#define DMS_FATIGUE_WARN            0x01
#define DMS_CALLING_WARN            0x02
#define DMS_SMOKING_WARN            0x03
#define DMS_DISTRACT_WARN           0x04
#define DMS_ABNORMAL_WARN           0x05

#define DMS_SANPSHOT_EVENT          0x10
#define DMS_DRIVER_CHANGE           0x11

typedef struct __DsmWarnFrame {
    uint32_t	warning_id;
    uint8_t		status_flag;
    uint8_t		sound_type;
    uint8_t		FatigueVal;
    uint8_t		resv[4];

    uint8_t		car_speed;
    uint16_t	altitude; //海拔
    uint32_t	latitude; //纬度
    uint32_t	longitude; //经度

    uint8_t		time[6];
    CarStatus	car_status;	
    uint8_t		mm_num;
    SBMmHeader mm[0];

} __attribute__((packed)) DsmWarnFrame;

//para
typedef struct __AdasParaSetting{
    uint8_t warning_speed_val;
    uint8_t warning_volume;
    uint8_t auto_photo_mode;
    uint16_t auto_photo_time_period;
    uint16_t auto_photo_distance_period;
    uint8_t photo_num;
    uint8_t photo_time_period;
    uint8_t image_Resolution;
    uint8_t video_Resolution;
    uint8_t reserve[9];

    uint8_t obstacle_distance_threshold;
    uint8_t obstacle_video_time;
    uint8_t obstacle_photo_num;
    uint8_t obstacle_photo_time_period;

    uint8_t FLC_time_threshold;
    uint8_t FLC_times_threshold;
    uint8_t FLC_video_time;
    uint8_t FLC_photo_num;
    uint8_t FLC_photo_time_period;

    uint8_t LDW_video_time;
    uint8_t LDW_photo_num;
    uint8_t LDW_photo_time_period;


    uint8_t FCW_time_threshold;
    uint8_t FCW_video_time;
    uint8_t FCW_photo_num;
    uint8_t FCW_photo_time_period;


    uint8_t PCW_time_threshold;
    uint8_t PCW_video_time;
    uint8_t PCW_photo_num;
    uint8_t PCW_photo_time_period;

    uint8_t HW_time_threshold;
    uint8_t HW_video_time;
    uint8_t HW_photo_num;
    uint8_t HW_photo_time_period;

    uint8_t TSR_photo_num;
    uint8_t TSR_photo_time_period;

    uint8_t reserve2[4];
} __attribute__((packed)) AdasParaSetting;


typedef struct _DmsParaSetting{

    //uint8_t Warn_SpeedThreshold;
    uint8_t warning_speed_val;
    uint8_t warning_volume;

    uint8_t auto_photo_mode;
    uint16_t auto_photo_time_period;
    uint16_t auto_photo_distance_period;

    uint8_t photo_num;
    uint8_t photo_time_period;

    uint8_t image_Resolution;
    uint8_t video_Resolution;
    uint8_t reserve[10];

    uint16_t Smoke_TimeIntervalThreshold;
    uint16_t Call_TimeIntervalThreshold;

    uint8_t FatigueDriv_VideoTime;
    uint8_t FatigueDriv_PhotoNum;
    uint8_t FatigueDriv_PhotoInterval;
    uint8_t FatigueDriv_resv;

    uint8_t CallingDriv_VideoTime;
    uint8_t CallingDriv_PhotoNum;
    uint8_t CallingDriv_PhotoInterval;

    uint8_t SmokingDriv_VideoTime;
    uint8_t SmokingDriv_PhotoNum;
    uint8_t SmokingDriv_PhotoInterval;

    uint8_t DistractionDriv_VideoTime;
    uint8_t DistractionDriv_PhotoNum;
    uint8_t DistractionDriv_PhotoInterval;

    uint8_t AbnormalDriv_VideoTime;
    uint8_t AbnormalDriv_PhotoNum;
    uint8_t AbnormalDriv_PhotoInterval;

    uint8_t reserve2[2];
} __attribute__((packed)) DmsParaSetting;

// with tangdajun
typedef struct __DmsAlertInfo {
    //#ifdef BIG_ENDIAN
#if 0
#else /*Little Endian*/

    //短时间闭眼报警 在字节的第两位。

    /* 短时间闭眼报警 */
    uint8_t alert_eye_close1:2;
    /* 长时间闭眼报警 */
    uint8_t alert_eye_close2:2;
    /* 左顾右盼 */
    uint8_t alert_look_around:2;
    /* 打哈欠 */
    uint8_t alert_yawn:2;


    /* 打电话 */
    uint8_t alert_phone:2;
    /* 吸烟 */
    uint8_t alert_smoking:2;
    /* 离岗 */
    uint8_t alert_absence:2;
    /* 低头 */
    uint8_t alert_bow:2;

    /* 抬头 */
    uint8_t alert_look_up:2;
    /* 驾驶员变更 */
    uint8_t alert_faceId:2;
    uint8_t byte2_recv:4;

    uint8_t byte3_recv;
    uint8_t byte4_recv;
    uint8_t byte5_recv;
    uint8_t byte6_recv;
    uint8_t byte7_recv;

#endif
} __attribute__((packed)) DmsAlertInfo;
/*****************苏标 SB END**********************/


/****************Minieye CAN Frame*****************/
typedef struct __MECANWarningMessage {
    //#ifdef BIG_ENDIAN
#if 0
    uint8_t     byte0_resv:5;
    uint8_t     sound_type:3;

    uint8_t     byte1_resv0:2;
    uint8_t     zero_speed:1;
    uint8_t     byte1_resv1:5;

    uint8_t     headway_measurement:7;
    uint8_t     headway_valid:1;

    uint8_t     byte3_resv:7;
    uint8_t     no_error:1;

    uint8_t     byte4_resv:4;
    uint8_t     fcw_on:1;
    uint8_t     right_ldw:1;
    uint8_t     left_ldw:1;
    uint8_t     ldw_off:1;

    uint8_t     byte5_resv;
    uint8_t     byte6_resv;

    uint8_t     byte7_resv:6;
    uint8_t     headway_warning_level:2;
#else /*Little Endian*/
    uint8_t     sound_type:3;
    uint8_t     time_indicator:2;
    uint8_t     byte0_resv:3;

    uint8_t     byte1_resv1:5;
    uint8_t     zero_speed:1;
    uint8_t     byte1_resv0:2;

    uint8_t     headway_valid:1;
    uint8_t     headway_measurement:7;

    uint8_t     no_error:1;
    uint8_t     error_code:7;

    uint8_t     ldw_off:1;
    uint8_t     left_ldw:1;
    uint8_t     right_ldw:1;
    uint8_t     fcw_on:1;
    uint8_t     byte4_resv:2;
    uint8_t     maintenanc:1;
    uint8_t     failsafe:1;

    uint8_t     byte5_resv0:1;
    uint8_t     peds_fcw:1;
    uint8_t     peds_in_dz:1;
    uint8_t     byte5_resv1:2;
    uint8_t     tamper_alert:1;
    uint8_t     byte5_resv2:1;
    uint8_t     tsr_enable:1;

    uint8_t     tsr_warning_level:3;
    uint8_t     byte6_resv:5;

    uint8_t     headway_warning_level:2;
    uint8_t     hw_repeatable_enable:1;
    uint8_t     byte7_resv:5;
#endif
} __attribute__((packed)) MECANWarningMessage;

#if 1
#define HW_LEVEL_NO_CAR     (0)
#define HW_LEVEL_WHITE_CAR  (1)
#define HW_LEVEL_RED_CAR    (2)

#define SOUND_WARN_NONE     (0x0)
#define SOUND_TYPE_SILENCE  (0)
#define SOUND_TYPE_LLDW     (1)
#define SOUND_TYPE_RLDW     (2)
#define SOUND_TYPE_HW       (3)
#define SOUND_TYPE_TSR      (4)
#define SOUND_TYPE_VB       (5)
#define SOUND_TYPE_FCW_PCW  (6)
#endif



typedef struct __CAN760Info {
//#ifdef BIG_ENDIAN
#if 0
    uint8_t     byte0_resv1:1;
    uint8_t     byte0_resv0:1;
    uint8_t     high_beam:1;
    uint8_t     low_beam:1;
    uint8_t     wipers:1;
    uint8_t     right_signal:1;
    uint8_t     left_signal:1;
    uint8_t     brakes:1;

    uint8_t     speed_aval:1;
    uint8_t     byte1_resv1:1;
    uint8_t     high_beam_aval:1;
    uint8_t     low_beam_aval:1;
    uint8_t     wipers_aval:1;
    uint8_t     byte1_resv0:3;

    uint8_t     speed;

#else /*Little Endian*/
    uint8_t     brakes:1;
    uint8_t     left_signal:1;
    uint8_t     right_signal:1;
    uint8_t     wipers:1;
    uint8_t     low_beam:1;
    uint8_t     high_beam:1;
    uint8_t     byte0_resv0:1;
    uint8_t     byte0_resv1:1;

    uint8_t     byte1_resv0:3;
    uint8_t     wipers_aval:1;
    uint8_t     low_beam_aval:1;
    uint8_t     high_beam_aval:1;
    uint8_t     byte1_resv1:1;
    uint8_t     speed_aval:1;

    uint8_t     speed;

#endif
} __attribute__((packed)) CAN760Info;
/****************Minieye CAN Frame END*****************/





/*******************DMS Websocket to CAN Frame****************************/
typedef struct __DmsCan778 {
    uint8_t Left_Eyelid_fraction;
    uint8_t Right_Eyelid_fraction;
    uint8_t Head_Yaw;
    uint8_t Head_Pitch;
    uint8_t Head_Roll;
    uint8_t Frame_Tag;
    uint8_t reserved[2];
} __attribute__((packed)) DmsCan778;

typedef struct __DmsCan779 {
    uint8_t Eye_Closure_Warning:2;
    uint8_t Yawn_warning:2;
    uint8_t Look_around_warning:2;
    uint8_t Look_up_warning:2;

    uint8_t Look_down_warning:2;
    uint8_t Phone_call_warning:2;
    uint8_t Smoking_warning:2;
    uint8_t Absence_warning:2;

    uint8_t Frame_Tag;
    uint8_t reserved[5];

} __attribute__((packed)) DmsCan779;


typedef struct __DmsCanFrame {
    char can_779_valid;
    char can_778_valid;

    DmsCan779 can_779;
    DmsCan778 can_778;
} __attribute__((packed)) DmsCanFrame;

/*******************DMS Websocket to CAN Frame END****************************/




/*********************Websocket socket message****************************/
typedef struct __WsiFrame{
    uint8_t warning[8];
    char source[20];
    uint64_t time;
    char topic[20];
}WsiFrame;

/*********************Websocket socket message END****************************/



/*********************local config file****************************/
typedef struct __LocalConfig {
    
    char usingParaFile;

    char serverip[32];
    uint16_t serverport;

    char clientip[32];
    uint16_t clientport;
    char netdev_name[32];

    //uart
    char tty_name[32];
    int  baudrate;
    char parity[32];
    int  bits;
    int  stopbit;

    char jpeg_coder_fps; // 帧率设置
    char speed_filter_enable; //报警的速度阀值
    char alert_time_filter;  //报警过滤时间
    char record_speed;
    uint32_t record_period;

    char use_heart;
    uint32_t check_heart_period;

    uint32_t adasBitRate[7];
    uint32_t dmsBitRate[6];

} __attribute__((packed)) LocalConfig;
/*********************local config file END****************************/


typedef struct __MmInfo_node{
#define SLOT_STABLE     0
#define SLOT_WRITING    1
#define SLOT_READING    2
    char rw_flag;
    uint8_t devid;
    uint8_t warn_type;
    uint8_t mm_type;
    uint32_t mm_id;
    uint8_t time[6];
} __attribute__((packed)) MmInfo_node;

#define WARN_SNAP_NUM_MAX   10
typedef struct __InfoForStore{

    uint8_t get_another_camera_video;
    uint8_t warn_type;
    //uint8_t mm_type;
    uint8_t photo_enable;
    uint8_t sound_enable;
    uint8_t video_enable;

    uint8_t video_time;
    uint8_t photo_time_period;
    uint8_t photo_num;
    uint32_t photo_id[WARN_SNAP_NUM_MAX];
    uint32_t video_id[2];
} __attribute__((packed)) InfoForStore;

//pthread
void *pthread_websocket_client(void *para);
void *pthread_process_recv(void *para);
void *pthread_encode_jpeg(void *p);
void *pthread_save_media(void *p);
void *pthread_req_media_process(void *para);
void *pthread_snap_shot(void *p);
void *pthread_process_send(void *para);

//para file
int global_var_init();
void read_dev_para(void *para, uint8_t para_type);
void write_dev_para(void *para, uint8_t para_type);
int write_local_adas_para_file(const char* filename);
int write_local_dms_para_file(const char* filename);
void set_AdasParaSetting_default();
void set_DmsParaSetting_default();
void print_dms_para(DmsParaSetting *para);
void print_adas_para(AdasParaSetting *para);
int read_local_adas_para_file(const char* filename);
int read_local_dms_para_file(const char* filename);

int write_file(const char* filename, const void* data, size_t size);

//prot
int do_snap_shot();
char *warning_type_to_str(uint8_t type);
int deal_wsi_adas_can700(WsiFrame *sourcecan);
int deal_wsi_adas_can760(WsiFrame *sourcecan);
void adas_para_check(AdasParaSetting *para);

void get_latitude_info(char *buffer, int len);
void deal_wsi_dms_info( WsiFrame *can);
void deal_wsi_dms_info2( DmsCan779 *can);

void mmid_to_filename(uint32_t id, uint8_t type, char *filepath);


#define IMAGE_SIZE_PER_PACKET   (64*1024)
#define PTR_QUEUE_BUF_SIZE   (2*(IMAGE_SIZE_PER_PACKET + 64)) //加64, 大于 header + tail, 

#define DATA_BUF_SIZE IMAGE_SIZE_PER_PACKET
#define SLIP_DATA_BUF_SIZE PTR_QUEUE_BUF_SIZE

typedef struct __prot_handle{

    int fd;

    LocalConfig *config;
    uint8_t *rcvData_s; /*slip 封装数据*/
    uint8_t *rcvData; /*unslip 数据*/
    int rcvlen;

    uint8_t *sndData_s;
    uint8_t *sndData;
    int sndlen;

/*
 *              IO handle
 * */

    int (*snd)(struct __prot_handle *handle);
    int (*rcv)(struct __prot_handle *handle);
    int (*on_connect)(struct __prot_handle *handle);

    int (*init)(struct __prot_handle *handle);
    int (*close)(struct __prot_handle *handle);

    
    void (*parse)(struct __prot_handle *handle);

    //int (*do_rest)(LocalConfig *config);

}prot_handle;

#endif


