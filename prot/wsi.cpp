#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>

#include <syslog.h>
#include <sys/time.h>
#include <unistd.h>

#include <libwebsockets.h>
#include <msgpack.h>
#include <sys/prctl.h>
#include <queue>

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include <cstdio>

#include "common/hal/halio.h"
#include "common/hal/canio.h"

#include "common/hal/android_gsensor.h"
#include "common/hal/tty.h"
#include "common/hal/halio.h"
#include "common/hal/esr.h"
#include "common/hal/fmu.h"
#include "common/hal/wrs.h"
#include "common/hal/canio.h"
#include "common/hal/camctl.h"


#include "prot.h"
#include "common.h"

using namespace rapidjson;
using namespace std;


#define MOBILEYE_WARNING_ID     (0x700)
#define MOBILEYE_CARSIGNAL_ID   (0x760)

/*******xuhan interface*************************/
#define DMS_INFO_TOPIC  ("output.info.v1")
/*******tangdajun interface*************************/
#define DMS_ALERT_0X100 "dms.alert.0x100"

#define MESSAGE_CAN700	"output.can.0x700"
#define MESSAGE_CAN760	"output.can.0x760"

/**************
 *  topic: subscribe
 *  source: client-name
 *  data: dms.alert.0x100
 * ****************/
static int pack_wsi_msg(uint8_t *data, uint32_t len, const char *name)
{
    unsigned int map_size = 3;
    uint32_t minlen = 0;

    if(!data || len < 0)
        return -1;

    msgpack_sbuffer sbuf;
    msgpack_sbuffer_init(&sbuf);
    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
    msgpack_pack_map(&pk, map_size);

#define PACK_MAP_MSG(type, type_len, content, content_len)\
    msgpack_pack_str(&pk, type_len);\
    msgpack_pack_str_body(&pk, type, type_len);\
    msgpack_pack_str(&pk, content_len);\
    msgpack_pack_str_body(&pk, content, content_len);

    PACK_MAP_MSG("topic", strlen("topic"), "subscribe", strlen("subscribe"));
    PACK_MAP_MSG("source", strlen("source"), "client-name", strlen("client-name"));

    PACK_MAP_MSG("data", strlen("data"), name, strlen(name));

    minlen = (sbuf.size<len) ? sbuf.size : len;
    memcpy(data, sbuf.data, minlen);

    msgpack_sbuffer_destroy(&sbuf);

    return minlen;
}

#if 0
/*********for debug**********/
const char* type_to_str(uint8_t type)
{
    switch(type){
        case MSGPACK_OBJECT_NIL:
            return "OBJECT_NIL";
        case MSGPACK_OBJECT_BOOLEAN:
            return "BOOLEAN";
        case MSGPACK_OBJECT_POSITIVE_INTEGER:
            return "POSITIVE_INTEGER";
        case MSGPACK_OBJECT_NEGATIVE_INTEGER:
            return "NEGATIVE_INTEGER";
        case MSGPACK_OBJECT_FLOAT32:
            return "FLOAT32";
        case MSGPACK_OBJECT_FLOAT64:
            return "FLOAT64";
        case MSGPACK_OBJECT_EXT:
            return "EXT";
        case MSGPACK_OBJECT_ARRAY:
            return "ARRAY";

        case MSGPACK_OBJECT_BIN:
            return "BIN";
        case MSGPACK_OBJECT_STR:
            return "STR";
        case MSGPACK_OBJECT_MAP:
            return "MAP";
        default:
            return "DEFAULT";
    }
}
#endif

static void msgpack_object_get(FILE* out, msgpack_object o, WsiFrame *can)
{
    enum MSG_DATA_TYPE{
        CAN_DATA=1,
        CAN_SOURCE,
        CAN_TIME,
        CAN_TOPIC,
    };
    static char data_type = 0;

    //WSI_DEBUG("msgpack type = %d\n", o.type);
    switch(o.type) {
        case MSGPACK_OBJECT_NIL:
        case MSGPACK_OBJECT_BOOLEAN:
        case MSGPACK_OBJECT_POSITIVE_INTEGER:
        case MSGPACK_OBJECT_NEGATIVE_INTEGER:
        case MSGPACK_OBJECT_FLOAT32:
        case MSGPACK_OBJECT_FLOAT64:
        case MSGPACK_OBJECT_EXT:
        case MSGPACK_OBJECT_ARRAY:
            if(data_type == CAN_TIME)
            {
                data_type = 0;
                //printf("get uint64: %ld\n", o.via.u64);
                can->time = o.via.u64;
                break;
            }
            break;
        case MSGPACK_OBJECT_BIN:
            //printf("get bin: %d\n", o.via.bin.size);
            //printbuf((uint8_t *)o.via.bin.ptr,o.via.bin.size);
            //printf("get bin: %d\n", o.via.i64);

            if(data_type == CAN_DATA)
            {
                data_type = 0;
                //printf("data enter!\n");
                memcpy(can->warning, o.via.bin.ptr, (o.via.bin.size > sizeof(can->warning) ? sizeof(can->warning) : o.via.bin.size));
                break;
            }
            break;

        case MSGPACK_OBJECT_STR:
            //printf("get str: %s\n", o.via.str.ptr);

            if(data_type == CAN_SOURCE)
            {	
                data_type = 0;
                memcpy(can->source, o.via.str.ptr, (o.via.str.size > sizeof(can->source) ? sizeof(can->source) : o.via.str.size));
                break;
            }
            else if(data_type == CAN_TOPIC)
            {
                //printf("topic enter!\n");
                data_type = 0;
                memcpy(can->topic, o.via.str.ptr, (o.via.str.size > sizeof(can->topic) ? sizeof(can->topic) : o.via.str.size));
                break;
            }
            else
            {
                if(!strncmp(o.via.str.ptr, "data", strlen("data")))
                {
                    data_type = CAN_DATA;
                    break;
                }
                if(!strncmp(o.via.str.ptr, "source", strlen("source")))
                {
                    data_type = CAN_SOURCE;
                    break;
                }
                if(!strncmp(o.via.str.ptr, "time", strlen("time")))
                {
                    data_type = CAN_TIME;
                    break;
                }
                if(!strncmp(o.via.str.ptr, "topic", strlen("topic")))
                {
                    data_type = CAN_TOPIC;
                    break;
                }
            }
            break;

        case MSGPACK_OBJECT_MAP:
            if(o.via.map.size != 0) {
                msgpack_object_kv* p = o.via.map.ptr;
                msgpack_object_kv* const pend = o.via.map.ptr + o.via.map.size;
                msgpack_object_get(out, p->key, NULL);
                msgpack_object_get(out, p->val, can);
                ++p;
                for(; p < pend; ++p) {
                    msgpack_object_get(out, p->key, NULL);
                    msgpack_object_get(out, p->val, can);
                }
            }
            break;

        default:
            if (o.via.u64 > ULONG_MAX)
                fprintf(out, "#<UNKNOWN %i over 4294967295>", o.type);
            else
                fprintf(out, "#<UNKNOWN %i %lu>", o.type, (unsigned long)o.via.u64);
    }
    return;
}
// use to get "output.info.v1" message
static int msgpack_object_dms_get(FILE* out, msgpack_object o, WsiFrame *can, uint8_t *buf, uint32_t *buflen)
{
    enum MSG_DATA_TYPE{
        CAN_DATA=1,
        CAN_SOURCE,
        CAN_TIME,
        CAN_TOPIC,
    };
    static char data_type = 0;

    //printf("type = %s\n", type_to_str(o.type));
    switch(o.type) {
        case MSGPACK_OBJECT_NIL:
        case MSGPACK_OBJECT_BOOLEAN:
        case MSGPACK_OBJECT_POSITIVE_INTEGER:
        case MSGPACK_OBJECT_NEGATIVE_INTEGER:
        case MSGPACK_OBJECT_FLOAT32:
        case MSGPACK_OBJECT_FLOAT64:
        case MSGPACK_OBJECT_EXT:
        case MSGPACK_OBJECT_ARRAY:
            //case MSGPACK_OBJECT_BIN:
            //printf("get uint64: %ld\n", o.via.u64);
            //printf("get bin: %d\n", o.via.i64);
            if(data_type == CAN_TIME)
            {
                data_type = 0;
                //printf("get uint64: %ld\n", o.via.u64);
                can->time = o.via.u64;
                break;
            }
            break;
        case MSGPACK_OBJECT_BIN:
            //printf("get bin: %d\n", o.via.bin.size);
            //printbuf((uint8_t *)o.via.bin.ptr,o.via.bin.size);
            //printf("get bin: %d\n", o.via.i64);

            if(data_type == CAN_DATA)
            {
                data_type = 0;
                //memcpy(can->warning, o.via.bin.ptr, (o.via.bin.size > sizeof(can->warning) ? sizeof(can->warning) : o.via.bin.size));
                *buflen = o.via.bin.size > *buflen ? *buflen : o.via.bin.size;
                //printf("data enter! len = %d / %d\n", o.via.bin.size, *buflen);
                memcpy(buf, o.via.bin.ptr, *buflen);
                break;
            }
            break;

        case MSGPACK_OBJECT_STR:
            //printf("get str: %s\n", o.via.str.ptr);

            if(data_type == CAN_SOURCE)
            {	
                data_type = 0;
                memcpy(can->source, o.via.str.ptr, (o.via.str.size > sizeof(can->source) ? sizeof(can->source) : o.via.str.size));
                break;
            }
            else if(data_type == CAN_TOPIC)
            {
                //printf("topic enter!\n");
                data_type = 0;
                memcpy(can->topic, o.via.str.ptr, (o.via.str.size > sizeof(can->topic) ? sizeof(can->topic) : o.via.str.size));
                break;
            }
            else
            {
                if(!strncmp(o.via.str.ptr, "data", strlen("data")))
                {
                    data_type = CAN_DATA;
                    break;
                }
                if(!strncmp(o.via.str.ptr, "source", strlen("source")))
                {
                    data_type = CAN_SOURCE;
                    break;
                }
                if(!strncmp(o.via.str.ptr, "time", strlen("time")))
                {
                    data_type = CAN_TIME;
                    break;
                }
                if(!strncmp(o.via.str.ptr, "topic", strlen("topic")))
                {
                    data_type = CAN_TOPIC;
                    break;
                }
            }
            break;

        case MSGPACK_OBJECT_MAP:
            if(o.via.map.size != 0) {
                msgpack_object_kv* p = o.via.map.ptr;
                msgpack_object_kv* const pend = o.via.map.ptr + o.via.map.size;
                msgpack_object_dms_get(out, p->key, NULL, buf, buflen);
                msgpack_object_dms_get(out, p->val, can, buf, buflen);
                ++p;
                for(; p < pend; ++p) {
                    msgpack_object_dms_get(out, p->key, NULL, buf, buflen);
                    msgpack_object_dms_get(out, p->val, can, buf, buflen);
                }
            }
            break;

        default:
            if (o.via.u64 > ULONG_MAX)
                fprintf(out, "#<UNKNOWN %i over 4294967295>", o.type);
            else
                fprintf(out, "#<UNKNOWN %i %lu>", o.type, (unsigned long)o.via.u64);
    }
    return 0;
}

static int get_dms_alert(const rapidjson::Value& val)
{
    assert(val["score"].IsNumber());
    assert(val["score"].IsDouble());
    assert(val["alerting"].IsBool());
    assert(val.HasMember("alerting"));

    if (val.HasMember("score")) {
        WSI_DEBUG("score = %f\n", val["score"].GetDouble());
    }   

    if(val["score"].GetDouble() == 0){
        WSI_DEBUG("no detect!\n");
        return 0;
    }

    //is alert
    if(val["alerting"].GetBool()){
        WSI_DEBUG("alert!\n");
        return 2;
    }else{
        WSI_DEBUG("detect!\n");
        return 1;
    }
}

static bool get_dms_pose(const rapidjson::Value& val, uint8_t *yaw, uint8_t *pitch, uint8_t *roll)
{
    assert(val["yaw"].IsNumber());
    assert(val["yaw"].IsDouble());
    *yaw = (uint8_t)(val["yaw"].GetDouble() + 127);
    WSI_DEBUG("yaw = %g\n", val["yaw"].GetDouble());

    assert(val["pitch"].IsNumber());
    assert(val["pitch"].IsDouble());
    *pitch = (uint8_t)(val["pitch"].GetDouble() + 127);
    WSI_DEBUG("pitch = %g\n", val["pitch"].GetDouble());

    assert(val["roll"].IsNumber());
    assert(val["roll"].IsDouble());
    *roll= (uint8_t)(val["roll"].GetDouble() + 127);
    WSI_DEBUG("roll = %g\n", val["roll"].GetDouble());

    return true;
}

#if 0
static int adas_parse_data_json(char *buffer)
{
    Document document;  // Default template parameter uses UTF8 and MemoryPoolAllocator.

    // In-situ parsing, decode strings directly in the source string. Source must be string.
    if (document.ParseInsitu(buffer).HasParseError()){
        printf("Parsing to document error\n");
        return 1;
    }

    WSI_DEBUG("Parsing to document succeeded.\n");
    assert(document.IsObject()); 
    assert(document.HasMember("topic"));
    assert(document.HasMember("source"));
    assert(document.HasMember("time"));
    assert(document.HasMember("data"));


    assert(document["topic"].IsString());
    printf("topic = %s\n", document["topic"].GetString());
    assert(document["source"].IsString());
    printf("source = %s\n", document["source"].GetString());

    assert(document["time"].IsNumber());   // Number is a JSON type, but C++ needs more specific type.
    assert(document["time"].IsUint64());      // In this case, IsUint()/IsInt64()/IsUInt64() also return true.
    printf("time = %ld\n", document["time"].GetUint64()); // Alternative (int)document["i"]

    assert(document["data"].IsNumber());   // Number is a JSON type, but C++ needs more specific type.
    assert(document["data"].IsUint64());      // In this case, IsUint()/IsInt64()/IsUInt64() also return true.
    printf("data = %ld\n", document["data"].GetUint64()); // Alternative (int)document["i"]

    return 0;
}
#endif

static int dms_parse_data_json(char *buffer, DmsCanFrame *can_frame)
{
    Document document;  // Default template parameter uses UTF8 and MemoryPoolAllocator.
    static uint32_t s_frame_tag_778 = 0, s_frame_tag_779 = 0;
    uint8_t yaw = 0, pitch = 0, roll = 0;

    // In-situ parsing, decode strings directly in the source string. Source must be string.
    if (document.ParseInsitu(buffer).HasParseError())
        return 1;

    WSI_DEBUG("Parsing to document succeeded.\n");
    assert(document.IsObject()); 
    assert(document.HasMember("eye_alert"));
    assert(document.HasMember("yawn_alert"));
    assert(document.HasMember("look_around_alert"));
    assert(document.HasMember("look_up_alert"));
    assert(document.HasMember("look_down_alert"));
    assert(document.HasMember("phone_alert"));
    assert(document.HasMember("smoking_alert"));
    assert(document.HasMember("absence_alert"));

    //build can 779
    can_frame->can_779.Eye_Closure_Warning = get_dms_alert(document["eye_alert"]);
    can_frame->can_779.Yawn_warning = get_dms_alert(document["yawn_alert"]);
    can_frame->can_779.Look_around_warning = get_dms_alert(document["look_around_alert"]);
    can_frame->can_779.Look_up_warning = get_dms_alert(document["look_up_alert"]);
    can_frame->can_779.Look_down_warning = get_dms_alert(document["look_down_alert"]);
    can_frame->can_779.Phone_call_warning = get_dms_alert(document["phone_alert"]);
    can_frame->can_779.Smoking_warning = get_dms_alert(document["smoking_alert"]);
    can_frame->can_779.Absence_warning = get_dms_alert(document["absence_alert"]);

    can_frame->can_779.Frame_Tag = s_frame_tag_779 & 0xFF;
    s_frame_tag_779 ++;
    memset(can_frame->can_779.reserved, 0, sizeof(can_frame->can_779.reserved));
    can_frame->can_779_valid = 1;

    //build can 778
    assert(document["face_detected"].IsBool());
    if(document["face_detected"].GetBool()){
        WSI_DEBUG("face detected!\n");
        assert(document["left_eye_open_faction"].IsNumber());
        assert(document["left_eye_open_faction"].IsDouble());
        can_frame->can_778.Left_Eyelid_fraction = (uint8_t)(document["left_eye_open_faction"].GetDouble()*100);

        assert(document["right_eye_open_faction"].IsNumber());
        assert(document["right_eye_open_faction"].IsDouble());
        can_frame->can_778.Right_Eyelid_fraction = (uint8_t)(document["right_eye_open_faction"].GetDouble()*100);

        assert(document.HasMember("intrinsic_pose"));
        assert(document["intrinsic_pose"].HasMember("yaw"));
        assert(document["intrinsic_pose"].HasMember("pitch"));
        assert(document["intrinsic_pose"].HasMember("roll"));
        get_dms_pose(document["intrinsic_pose"], &yaw, &pitch, &roll);
        can_frame->can_778.Head_Yaw = yaw;
        can_frame->can_778.Head_Pitch = pitch;
        can_frame->can_778.Head_Roll = roll;
        memset(can_frame->can_778.reserved, 0, sizeof(can_frame->can_778.reserved));
        can_frame->can_778.Frame_Tag = s_frame_tag_778 & 0xFF;
        s_frame_tag_778 ++;
        can_frame->can_778_valid = 1;
    }
    return 0;
}

int unpack_wsi_msg(char *data, size_t size)
{
#define MSGPACK_LEN (128*1024)
    msgpack_zone mempool;
    msgpack_object deserialized;
    WsiFrame can;

    msgpack_zone_init(&mempool, MSGPACK_LEN);
    msgpack_unpack((const char *)data, size, NULL, &mempool, &deserialized);
    memset(&can, 0, sizeof(can));
    msgpack_object_get(stdout, deserialized, &can);

    //msgpack_object_print(stdout, deserialized);
#if defined ENABLE_ADAS 
    if(!memcmp(can.topic, MESSAGE_CAN700, strlen(MESSAGE_CAN700))){
        //printf("700 come********************************\n");
        deal_wsi_adas_can700(&can);
    }else if(!memcmp(can.topic, MESSAGE_CAN760, strlen(MESSAGE_CAN760))){
        //printf("760 come********************************\n");
        deal_wsi_adas_can760(&can);
    }else{
        ;
    }
#elif defined ENABLE_DMS
    deal_wsi_dms_info(&can);
#endif
    msgpack_zone_destroy(&mempool);
    return 0;
}

//for dms new interface
#if 0
int unpack_wsi_msg2(char *data, size_t size)
{
#define DMS_JSON_MSG_LEN (1024*1024)
    DmsCanFrame can_frame;
    uint32_t len = 0;
    msgpack_zone mempool;
    msgpack_object deserialized;
    WsiFrame can;
    uint8_t data_msg[8192];

#if defined ENABLE_ADAS 
    msgpack_zone_init(&mempool, DMS_JSON_MSG_LEN);//1M
    msgpack_unpack((const char *)data, size, NULL, &mempool, &deserialized);
    memset(&can, 0, sizeof(can));

    msgpack_object_get(stdout, deserialized, &can);
    msgpack_zone_destroy(&mempool);

    if(!memcmp(can.topic, MESSAGE_CAN700, strlen(MESSAGE_CAN700))){
        //printf("700 come********************************\n");
        deal_wsi_adas_can700(&can);
    }else if(!memcmp(can.topic, MESSAGE_CAN760, strlen(MESSAGE_CAN760))){
        //printf("760 come********************************\n");
        deal_wsi_adas_can760(&can);
    }else{
        ;
    }
#elif defined ENABLE_DMS
    msgpack_zone_init(&mempool, DMS_JSON_MSG_LEN);//1M
    msgpack_unpack((const char *)data, size, NULL, &mempool, &deserialized);
    len = sizeof(data_msg);
    msgpack_object_dms_get(stdout, deserialized, &can, data_msg, &len);
    WSI_DEBUG("get bin data len = %d\n", len);

    //second unpack
    char *buffer = (char *)malloc(DMS_JSON_MSG_LEN);
    msgpack_unpack((const char *)data_msg, len, NULL, &mempool, &deserialized);
    msgpack_object_print_buffer(buffer, DMS_JSON_MSG_LEN, deserialized);
    msgpack_zone_destroy(&mempool);
    WSI_DEBUG("unpack:\n %s\n", buffer);

    memset(&can_frame, 0, sizeof(DmsCanFrame));
    dms_parse_data_json(buffer, &can_frame);
    if(can_frame.can_779_valid){
        WSI_DEBUG("can779:\n");
        //printbuf(&can_frame.can_779, sizeof(DmsCan779));
        //halio.send_can_frame(0x779, (char *)&can_frame.can_779);
        deal_wsi_dms_info2(&can_frame.can_779);
    }
    if(can_frame.can_778_valid){
        WSI_DEBUG("can778:\n");
        //printbuf(&can_frame.can_778, sizeof(DmsCan778));
        //halio.send_can_frame(0x778, (char *)&can_frame.can_778);
    }
    if(buffer)
        free(buffer);
#endif
    return 0;
}
#endif

volatile int force_exit = 0;
static struct lws *wsi_dumb;
enum demo_protocols {
    PROTOCOL_LWS_TO_CAN,

    /* always last */
    DEMO_PROTOCOL_COUNT
};


#if defined ENABLE_ADAS 
enum Subscribe_Msg {
    MESSAGE_INDEX_MIN,
    MESSAGE_CAN700_INDEX,
    MESSAGE_CAN760_INDEX,

    /* always last */
    MESSAGE_INDEX_MAX
};
#elif defined ENABLE_DMS
enum Subscribe_Msg {
    MESSAGE_INDEX_MIN,

    MESSAGE_CANDMS_INDEX,

    /* always last */
    MESSAGE_INDEX_MAX
};
#endif

/*********for debug**********/
#if 0 
static void write_wsi_log(int reasons)
{
    FILE *fp;
    char buf[50];
    int len = 0;

    switch(reasons){
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            sprintf(buf, "%s\n", "CLIENT_ESTABLISHED");
            break;
        case LWS_CALLBACK_CLOSED:
            sprintf(buf, "%s\n", "CLIENT_CLOSED");
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:
            sprintf(buf, "%s\n", "CLIENT_RECV");
            break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            sprintf(buf, "%s\n", "ONNECTION_ERROR");
            break;
        case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
            sprintf(buf, "%s\n", "CLIENT_CONFIRM_EXTENSION_SUPPORTED");
            break;
        case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
            sprintf(buf, "%s\n", "ESTABLISHED_CLIENT_HTTP");
            break;
        case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
            sprintf(buf, "%s\n", "RECEIVE_CLIENT_HTTP_READ");
            break;
        case LWS_CALLBACK_RECEIVE_CLIENT_HTTP:
            sprintf(buf, "%s\n", "RECEIVE_CLIENT_HTTP");
            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            sprintf(buf, "%s\n", "CLIENT_WRITEABLE");
            break;
        case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
            sprintf(buf, "%s\n", "CLIENT_APPEND_HANDSHAKE_HEADER");
            break;
        case LWS_CALLBACK_CLIENT_HTTP_WRITEABLE:
            sprintf(buf, "%s\n", "CLIENT_HTTP_WRITEABLE");
            break;
        case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
            sprintf(buf, "%s\n", "COMPLETED_CLIENT_HTTP");
            break;
        case LWS_CALLBACK_PROTOCOL_DESTROY:
            sprintf(buf, "%s\n", "PROTOCOL_DESTROY");
            break;
        case LWS_CALLBACK_WSI_DESTROY:
            sprintf(buf, "%s\n", "WSI_DESTROY");
            break;
        default:
            sprintf(buf, " reason = %d, %s\n", reasons, "unknow reason");
            break;

    }

    fp = fopen("/mnt/obb/wsi.log", "a");
    len = strlen(buf);
    fwrite(buf, 1, len, fp);
    fclose(fp);
}
#endif

static int callback_lws_communicate(struct lws *wsi, enum lws_callback_reasons reason,
        void *user, void *in, size_t len)
{
#define LWS_WRITE_BUF_LEN   512
    const char *which = "http";
    char buf[LWS_WRITE_BUF_LEN + LWS_PRE];
    int n=0, msgpacklen=0;
    uint8_t datacmd[512];
    static int msg_send_index = MESSAGE_INDEX_MIN;

    //	printf("reason: %d\n", reason);
    //write_wsi_log(reason);
    switch (reason) {

        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("dumb:: LWS_CALLBACK_CLIENT_ESTABLISHED\n");
            lws_callback_on_writable(wsi);
            break;

        case LWS_CALLBACK_CLOSED:
            printf("dumb:: LWS_CALLBACK_CLOSED\n");
            msg_send_index = MESSAGE_INDEX_MIN;
            wsi_dumb = NULL;
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            //printf("receive websocket: %ld\n", len);
            //printbuf((uint8_t *)in, (int)len);
            unpack_wsi_msg((char *)in, len);

            //dms new interface
            //unpack_wsi_msg2((char *)in, len);
            break;

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            wsi_dumb = NULL;
            //printf("dumb:: LWS_CALLBACK_ERROR!\n");
            lwsl_err("CLIENT_CONNECTION_ERROR: %s: %s\n", which,
                    in ? (char *)in : "(null)");
            break;

        case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
            if ((strcmp((const char *)in, "x-webkit-deflate-frame") == 0))
                return 1;
            if ((strcmp((const char *)in, "deflate-frame") == 0))
                return 1;
            break;

        case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
            //printf("LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP\n");
            lwsl_notice("lws_http_client_http_response %d\n",
                    lws_http_client_http_response(wsi));
            break;

            /* chunked content */
        case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
            lwsl_notice("LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ: %ld\n",
                    (long)len);
            break;

            /* unchunked content */
        case LWS_CALLBACK_RECEIVE_CLIENT_HTTP:
            {
                char buffer[1024 + LWS_PRE];
                char *px = buffer + LWS_PRE;
                int lenx = sizeof(buffer) - LWS_PRE;

                if (lws_http_client_read(wsi, &px, &lenx) < 0)
                    return -1;
            }
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            printf("LWS_CALLBACK_CLIENT_WRITEABLE\n");
#if defined ENABLE_ADAS
           // printf("send %d\n", msg_send_index);
            if(msg_send_index == MESSAGE_CAN700_INDEX){
                printf("send 700\n");
                msgpacklen = pack_wsi_msg(datacmd, sizeof(datacmd), MESSAGE_CAN700);
                msg_send_index++;
            }else if(msg_send_index == MESSAGE_CAN760_INDEX){
                printf("send 700\n");
                msgpacklen = pack_wsi_msg(datacmd, sizeof(datacmd), MESSAGE_CAN760);
                msg_send_index++;
            }else if(msg_send_index < MESSAGE_INDEX_MAX){
                msg_send_index++;
                break;
            }else{
                break;
            }
#elif defined ENABLE_DMS
            if(msg_send_index == MESSAGE_CANDMS_INDEX){
                printf("request %s\n", DMS_ALERT_0X100);
                msgpacklen = pack_wsi_msg(datacmd, sizeof(datacmd), DMS_ALERT_0X100);
                msg_send_index++;
            }else if(msg_send_index < MESSAGE_INDEX_MAX){
                    msg_send_index++;
                    break;
            }else{
                break;
            }
#endif
            //printf("client send request, ret = %d!\n", msgpacklen);
            if(msgpacklen < LWS_WRITE_BUF_LEN && msgpacklen >0){
                memcpy(buf + LWS_PRE, datacmd, msgpacklen);
                n = lws_write(wsi, (unsigned char *)&buf[LWS_PRE], msgpacklen, LWS_WRITE_BINARY);
                if(n<=0){
                    printf("lws write ret = %d\n", n);
                }
            }
            lws_callback_on_writable(wsi);
            break;

        case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
            printf("LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER\n");
            break;

        case LWS_CALLBACK_CLIENT_HTTP_WRITEABLE:
            break;

        case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
            //   force_exit = 1;
            break;

        case LWS_CALLBACK_PROTOCOL_DESTROY:
            printf("protocol destroy!\n");
            break;

        case LWS_CALLBACK_WSI_DESTROY:
            printf("wsi destroy!!!\n");
            msg_send_index = MESSAGE_INDEX_MIN;
            wsi_dumb = NULL;
            break;

        default:
            break;
    }

    return 0;
}

/* list of supported protocols and callbacks */
static const struct lws_protocols protocols[] = {
    {
        "lws-to-can-protocol",
        callback_lws_communicate,
        0,
        8192, //set recv len max
    },
    { NULL, NULL, 0, 0 } /* end */
};


void notice_tcp_send_exit();
void pthread_exit_notice(void)
{
    notice_tcp_send_exit();
}

void sighandler(int sig)
{
    force_exit = 1;
    pthread_exit_notice();
    //exit(0);
}
#if 0
int ratelimit_connects(unsigned int *last, unsigned int secs)
{
    struct timeval tv; 

    gettimeofday(&tv, NULL);

    if (tv.tv_sec - (*last) < secs)
        return 0;

    *last = tv.tv_sec;

    return 1;
}
#else
int ratelimit_connects(unsigned int *last, unsigned int secs)
{
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);

    if (tv.tv_sec - (*last) < secs)
        return 0;

    *last = tv.tv_sec;

    return 1;
}
#endif
void *pthread_websocket_client(void *para)
{
    int port = 7681, use_ssl = 0, ietf_version = -1;
    unsigned int rl_dumb = 0,  do_ws = 1, pp_secs = 0;
    struct lws_context_creation_info info;
    struct lws_client_connect_info i;
    struct lws_context *context;

    prctl(PR_SET_NAME, "websocket");
    memset(&info, 0, sizeof info);
    memset(&i, 0, sizeof(i));

#if defined ENABLE_ADAS
    i.port = 24012;
#elif defined ENABLE_DMS
    i.port = 24011;
#endif
    i.address = "127.0.0.1";
    i.path = "./";

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.ws_ping_pong_interval = pp_secs;
    //   info.extensions = exts;
    context = lws_create_context(&info);
    if (context == NULL) {
        fprintf(stderr, "Creating libwebsocket context failed\n");
        return NULL;
    }
    i.context = context;
    i.ssl_connection = use_ssl;
    i.host = i.address;
    i.origin = i.address;
    i.ietf_version_or_minus_one = ietf_version;

    while (!force_exit) {
        if(!wsi_dumb && ratelimit_connects(&rl_dumb, 2u))
        {
            i.pwsi = &wsi_dumb;
            i.protocol = protocols[PROTOCOL_LWS_TO_CAN].name;
            lws_client_connect_via_info(&i);
            printf("connecting to websocket!\n");
        }
        lws_service(context, 200);
    }
    lwsl_err("wsi Exiting\n");
    lws_context_destroy(context);

    pthread_exit(NULL);
}

static void usage(const char *exe_name)
{
    printf("Usage:%s <switches> [option]\n", exe_name);
    printf("\n");

    printf("#Existing collector switches:\n");
    printf("%-10s %s\n", "-h", "Show this help message");
    printf("%-10s %s\n", "-f", "using para file");
    printf("%-10s %s\n", "-v", "Show program version");
    printf("%-10s %s\n", "-V <level>", "Set program verbose level");

    printf("\n");
}


LocalConfig g_configini;
void set_local_config_default(LocalConfig *config)
{
#define SERVER_IP "192.168.100.100"
#define SERVER_PORT 8888

#define CLIENT_IP "192.168.100.211"
#define NETDEV_NAME "eth0"


    uint32_t dmsCodeRate[6]={250, 500, 800, 1024, 1024, 2048}; /* kB */
    uint32_t adasCodeRate[7]={250, 500, 800, 1024, 800, 1024, 2048}; /* kB */

    config->serverport = SERVER_PORT;
    snprintf(&config->serverip[0], sizeof(config->serverip), "%s", SERVER_IP);
    snprintf(&config->clientip[0], sizeof(config->clientip), "%s", CLIENT_IP);
    snprintf(&config->netdev_name[0], sizeof(config->netdev_name), "%s", NETDEV_NAME);

    config->jpeg_coder_fps = 5;
    config->speed_filter_enable = 1;
    config->record_speed = 1;
    config->record_period = 30;

    config->alert_time_filter=10;
    config->use_heart = 1;
    config->check_heart_period = 90;

    memcpy(config->adasBitRate, adasCodeRate, sizeof(adasCodeRate));
    memcpy(config->dmsBitRate, dmsCodeRate, sizeof(dmsCodeRate));

#define TTY_NAME "/dev/ttySC0"
    snprintf(&config->tty_name[0], sizeof(config->tty_name), "%s", TTY_NAME);
    config->baudrate = 115200;
    snprintf(&config->parity[0], sizeof(config->parity), "%s", "NONE");
    config->stopbit = 1;
    config->bits = 8;
}

void local_config_dump(LocalConfig *config)
{
    uint32_t i;
    printf("***********Local config dump****************\n");
    printf("usingParaFile: %d\n", config->usingParaFile);

    printf("serverip: %s\n", config->serverip);
    printf("serverport: %d\n", config->serverport);
    printf("clientip: %s\n", config->clientip);
    printf("client netdev_name: %s\n", config->netdev_name);

    printf("tty_name: %s\n", config->tty_name);
    printf("baudrate: %d\n", config->baudrate);
    printf("parity: %s\n", config->parity);
    printf("stopbit: %d\n", config->stopbit);
    printf("bits: %d\n", config->bits);

    printf("jpeg_coder_fps : %d\n", config->jpeg_coder_fps);
    printf("speed_filter_enable : %d\n", config->speed_filter_enable);
    printf("alert_time_filter: %d\n", config->alert_time_filter);

    printf("record_speed : %d\n", config->record_speed);
    printf("record_period : %d\n", config->record_period);

    printf("use_heart : %d\n", config->use_heart);
    printf("check_heart_period : %d\n", config->check_heart_period);
    
    printf("adas bitRate:\n");
    for(i=0; i<sizeof(config->adasBitRate)/sizeof(uint32_t); i++){
        printf("adasBitRate %d\n", config->adasBitRate[i]);
    }
    printf("dms bitRate:\n");
    for(i=0; i<sizeof(config->dmsBitRate)/sizeof(uint32_t); i++){
        printf("dmsBitRate %d\n", config->dmsBitRate[i]);
    }
    printf("***********Local config dump****************\n");
}

/*******************prot config using json**********************/
static bool get_server_config(const rapidjson::Value& val, LocalConfig *config)
{
    assert(val["ip"].IsString());
    val["ip"].GetString();
    snprintf(&config->serverip[0], sizeof(config->serverip), "%s", val["ip"].GetString());

    assert(val["port"].IsNumber());
    assert(val["port"].IsUint());
    config->serverport = val["port"].GetUint();

    return true;
}

static bool get_client_config(const rapidjson::Value& val, LocalConfig *config)
{
    assert(val["ip"].IsString());
    val["ip"].GetString();
    snprintf(&config->clientip[0], sizeof(config->clientip), "%s", val["ip"].GetString());

    assert(val["port"].IsNumber());
    assert(val["port"].IsUint());
    config->clientport = val["port"].GetUint();

    assert(val["device"].IsString());
    val["device"].GetString();
    snprintf(&config->netdev_name[0], sizeof(config->netdev_name), "%s", val["device"].GetString());

    return true;
}

static bool get_tty_config(const rapidjson::Value& val, LocalConfig *config)
{
    assert(val["tty_name"].IsString());
    val["tty_name"].GetString();
    snprintf(&config->tty_name[0], sizeof(config->tty_name), "%s", val["tty_name"].GetString());

    assert(val["baudrate"].IsNumber());
    assert(val["baudrate"].IsUint());
    config->baudrate = val["baudrate"].GetUint();

    assert(val["bits"].IsNumber());
    assert(val["bits"].IsUint());
    config->bits = val["bits"].GetUint();

    assert(val["stopbit"].IsNumber());
    assert(val["stopbit"].IsUint());
    config->stopbit = val["stopbit"].GetUint();


    assert(val["parity"].IsString());
    val["parity"].GetString();
    snprintf(&config->parity[0], sizeof(config->parity), "%s", val["parity"].GetString());

    return true;
}

static bool get_alert_para(const rapidjson::Value& val, LocalConfig *config)
{
    assert(val["speed_filter_enable"].IsBool());
    if(val["speed_filter_enable"].GetBool()){
        config->speed_filter_enable = 1;
    }else{
        config->speed_filter_enable = 0;
    }

    assert(val["jpeg_fps"].IsNumber());
    assert(val["jpeg_fps"].IsUint());
    if (val.HasMember("jpeg_fps")) {
        config->jpeg_coder_fps = val["jpeg_fps"].GetUint();
    }

    assert(val["record_speed"].IsBool());
    if(val["record_speed"].GetBool()){
        config->record_speed = 1;
    }else{
        config->record_speed = 0;
    }

    assert(val["record_period"].IsNumber());
    assert(val["record_period"].IsUint());
    if (val.HasMember("record_period")) {
        config->record_period = val["record_period"].GetUint();
    }

    assert(val["alert_time_filter"].IsNumber());
    assert(val["alert_time_filter"].IsUint());
    if (val.HasMember("alert_time_filter")) {
        config->alert_time_filter= val["alert_time_filter"].GetUint();
    }

    assert(val["use_heart"].IsBool());
    if(val["use_heart"].GetBool()){
        config->use_heart= 1;
    }else{
        config->use_heart= 0;
    }

    assert(val["check_heart_period"].IsNumber());
    assert(val["check_heart_period"].IsUint());
    if (val.HasMember("check_heart_period")) {
        config->check_heart_period= val["check_heart_period"].GetUint();
    }

    assert(val["adasBitRate"].IsArray());
    /* rapidjson uses SizeType instead of size_t. */
    for (SizeType i = 0; i < val["adasBitRate"].Size(); i++){
        printf("val[adasBitRate][%d] = %d\n", i, val["adasBitRate"][i].GetInt());
        config->adasBitRate[i] = val["adasBitRate"][i].GetInt();
    }

    assert(val["dmsBitRate"].IsArray());
    for (SizeType i = 0; i < val["dmsBitRate"].Size(); i++){
        printf("val[dmsBitRate][%d] = %d\n", i, val["dmsBitRate"][i].GetInt());
        config->dmsBitRate[i] = val["dmsBitRate"][i].GetInt();
    }

    return true;
}

static int parse_prot_json(char *buffer, LocalConfig *config)
{
    Document document;  // Default template parameter uses UTF8 and MemoryPoolAllocator.

    // In-situ parsing, decode strings directly in the source string. Source must be string.
    if (document.ParseInsitu(buffer).HasParseError()){
        printf("Parsing to json error\n");
        printf("%s\n", buffer);
        return 1;
    }

    WSI_DEBUG("Parsing to document succeeded.\n");
    assert(document.IsObject()); 
    assert(document.HasMember("server"));
    assert(document.HasMember("client"));
    assert(document.HasMember("alert_para"));
    assert(document.HasMember("tty"));

    get_server_config(document["server"], config);
    get_client_config(document["client"], config);
    get_alert_para(document["alert_para"], config);
    get_tty_config(document["tty"], config);

    return 0;
}

int local_config_init(char *filename)
{
    char buffer[1024];
    size_t size = 0;
    size_t ret = 0;
    FILE *fp;

    printf("using para file init\n");
    fp = fopen(filename, "r");
    if(!fp){
        fprintf(stderr, "open %s fail, error:%s!\n",\
                filename, strerror(errno));
        return -1;
    }else{
        rewind(fp);
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        rewind(fp);
        ret = fread(buffer, 1, size, fp);
        printf("read %s, len = %ld\n", filename, ret);
        buffer[ret] = 0;
        //printf("%s\n", buffer);
        fclose(fp);
        if(ret != size){
            printf("read file not complete\n");
            return -1;
        }
        if(parse_prot_json(buffer, &g_configini)){
            return -1;
        }
    }
    return 0;
}


#define GETOPT_OPT_STR  "hvf:V:"
int main(int argc, char **argv)
{
    pthread_t pth[10];
    int i =0;
    int opt;

    printf("---compile time %s %s---\n", __DATE__, __TIME__);

    signal(SIGINT, sighandler);
    if(global_var_init()){
        printf("init fail\n");
        exit(0);
    }

    while ((opt = getopt(argc, (char * const *)argv, GETOPT_OPT_STR)) != -1) {
        switch(opt) {
            default:
            case 'h':
                usage(argv[0]);
                printf("version: %s\n", SOFTWARE_VERSION);
                exit(0);
            case 'V':
                if (!optarg) {
                    printf("using default verbose = %d\n", atoi(optarg));
                } else {
                    printf("verbose = %d\n", atoi(optarg));
                }
                break;
            case 'f':
                if (!optarg) {
                    printf("input para file!\n");
                } else {
                    printf("parse filename: %s\n", optarg);
                    if(local_config_init(optarg)){
                        exit(0);
                    }
                    g_configini.usingParaFile = 1;
                }
                break;
            case 'v':
                //printf(SOFTWARE_VERSION"\n");
                printf("version: %s\n", SOFTWARE_VERSION);
                exit(0);
        }
    }

    if(!g_configini.usingParaFile){
        printf("Using default configini.\n");
        set_local_config_default(&g_configini);
    }
    local_config_dump(&g_configini);

    if(pthread_create(&pth[0], NULL, pthread_websocket_client, NULL)){
        printf("pthread_create fail!\n");
        return -1;
    }
    if(pthread_create(&pth[1], NULL, pthread_process_recv, NULL)){
        printf("pthread_create fail!\n");
        return -1;
    }
    pthread_detach(pth[1]);


    if(pthread_create(&pth[2], NULL, pthread_process_send, NULL)){
        printf("pthread_create fail!\n");
        return -1;
    }
    if(pthread_create(&pth[4], NULL, pthread_save_media, NULL)){
        printf("pthread_create fail!\n");
        return -1;
    }
    if(pthread_create(&pth[5], NULL, pthread_snap_shot, NULL)){
        printf("pthread_create fail!\n");
        return -1;
    }
    pthread_join(pth[0], NULL);
    printf("join %d\n", i++);
    //pthread_join(pth[1], NULL);
    //printf("join %d\n", i++);
    pthread_join(pth[2], NULL);
    printf("join %d\n", i++);
    pthread_join(pth[4], NULL);
    printf("join %d\n", i++);
    pthread_join(pth[5], NULL);
    printf("join %d\n", i++);
    return 0;
}

