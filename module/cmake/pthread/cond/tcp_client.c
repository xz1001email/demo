#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <stdlib.h>
#include <pthread.h>

#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)


#define PROTOCOL_USING_BIG_ENDIAN

#ifdef PROTOCOL_USING_BIG_ENDIAN
#define MY_HTONL(x)     htonl(x)
#define MY_HTONS(x)     htons(x)
#else
#define MY_HTONL(x)     (x)
#define MY_HTONS(x)     (x)
#endif

int hostsock;
int force_exit = 0;

static int try_connect()
{
#define HOST_SERVER_PORT (8888)
#define TCP_RECV_BUF (64*1024)
#define TCP_SEND_BUF (64*1024)
    int sock;
    int32_t ret = 0;
    int enable = 1;
    const char *server_ip = "192.168.100.102";
    //const char *server_ip = "192.168.100.105";
    struct sockaddr_in host_serv_addr;
    socklen_t optlen;
    int bufsize = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Create socket failed %s\n", strerror(errno));
        return -2;
    }
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    bufsize = TCP_RECV_BUF;
    optlen = sizeof(bufsize);
    ret = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, optlen);
    if(ret == -1){
        printf("%s:%d error\n", __FILE__, __LINE__);
        return -1;
    }
    bufsize = TCP_SEND_BUF;
    optlen = sizeof(bufsize);
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, optlen);
    if(ret == -1){
        printf("%s:%d error\n", __FILE__, __LINE__);
        return -1;
    }
    bufsize = 0;
    optlen = sizeof(bufsize);
    getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, &optlen);
    printf("get recv buf size = %d\n", bufsize);
    getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, &optlen);
    printf("get send buf size = %d\n", bufsize);

    memset(&host_serv_addr, 0, sizeof(host_serv_addr));
    host_serv_addr.sin_family = AF_INET;
    host_serv_addr.sin_port   = MY_HTONS(HOST_SERVER_PORT);
    ret = inet_aton(server_ip, &host_serv_addr.sin_addr);
    if (0 == ret) {
        printf("inet_aton failed %d %s\n", ret, strerror(errno));
        return -2;
    }

    while(1)
    {
        if( 0 == connect(sock, (struct sockaddr *)&host_serv_addr, sizeof(host_serv_addr))){
            printf("connect ok!\n");
            return sock;
        }
        else{
            sleep(1);
            printf("try connect!\n");
        }
    }
}

void *pthread_tcp_process(void *para)
{
#define TCP_READ_BUF_SIZE (64*1024)
#define RECV_HOST_DATA_BUF_SIZE (128*1024)
    int32_t ret = 0;
    int i=0;
    static int tcprecvcnt = 0;
    uint8_t *readbuf = NULL;
    uint8_t *writebuf = NULL;
    uint8_t *msgbuf = NULL;

    prctl(PR_SET_NAME, "tcp_process");

    msgbuf = (uint8_t *)malloc(RECV_HOST_DATA_BUF_SIZE);
    if(!msgbuf){
        perror("parse_host_cmd malloc");
        return NULL;
    }
    readbuf = (uint8_t *)malloc(TCP_READ_BUF_SIZE);
    if(!readbuf){
        perror("tcp readbuf malloc");
        goto out;
    }

connect_again:
    hostsock = try_connect();
    if(hostsock < 0)
        goto connect_again;

#if defined ENABLE_ADAS
    send_work_status(SAMPLE_DEVICE_ID_ADAS);
#elif defined ENABLE_DSM
    send_work_status(SAMPLE_DEVICE_ID_DSM);
#endif

    while (!force_exit) {
        ret = read(hostsock, readbuf, TCP_READ_BUF_SIZE);
        if (ret <= 0) {
            printf("read failed %d %s\n", ret, strerror(errno));
            close(hostsock);
            hostsock = -1;
            goto connect_again;

            //continue;
        }else{//write to buf
            //MY_DEBUG("recv raw cmd, tcprecvcnt = %d:\n", tcprecvcnt++);
            //printbuf(readbuf, ret);
            i=0;
            while(ret--)
            {
                sleep(1);
            }
        }
    }
out:
    printf("go out!\n");
    if(readbuf)
        free(readbuf);
    if(msgbuf)
        free(msgbuf);

    if(hostsock>0)
        close(hostsock);
    pthread_exit(NULL);
}



int main(void)
{
    pthread_t thr;
    void *res;
    int s;

    /* Start a thread and then send it a cancellation request */

    s = pthread_create(&thr, NULL, &pthread_tcp_process, NULL);
    if (s != 0)
        handle_error_en(s, "pthread_create");

    sleep(2);           /* Give thread a chance to get started */

    printf("main(): sending cancellation request\n");
    s = pthread_cancel(thr);
    if (s != 0)
        handle_error_en(s, "pthread_cancel");

    /* Join with thread to see what its exit status was */

    printf("enter join\n");
    s = pthread_join(thr, &res);
    if (s != 0)
        handle_error_en(s, "pthread_join");

    if (res == PTHREAD_CANCELED)
        printf("main(): thread was canceled\n");
    else
        printf("main(): thread wasn't canceled (shouldn't happen!)\n");
    exit(EXIT_SUCCESS);
}










