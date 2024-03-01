#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/uio.h>
#include<sys/ioctl.h>
#include<arpa/inet.h>
#include<net/if.h>
#include<errno.h>
#include<strings.h>
#include<string.h>
#include <netinet/if_ether.h>
#include <net/if.h>


#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <getopt.h>


#define INTERFAXENAME "lo"


/* 
 * log_pos: 日志更新的位置
 * logbuf: 执行的日志发送给client
 *
 * */
static int log_pos;
static char logbuf[2048];

int parse_cmd(int argc, char **argv)
{
    int c;
    int digit_optind = 0;

    /* 每次调用需要复位optind */
    optind = 1;

    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"add",     required_argument, 0,  0 },
            {"append",  no_argument,       0,  0 },
            {"delete",  required_argument, 0,  0 },
            {"verbose", no_argument,       0,  0 },
            {"create",  required_argument, 0, 'c'},
            {"file",    required_argument, 0,  0 },
            {0,         0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "abc:d:012", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 0:
                printf("option %s", long_options[option_index].name);
                if (optarg) {
                    log_pos = snprintf(logbuf, sizeof(logbuf), "%s", optarg);
                    printf(" with arg %s, log_pos %d\n", optarg, log_pos);
                }
                printf("\n");
                break;
            case 1:
                printf("--option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;

            case '0':
            case '1':
            case '2':
                if (digit_optind != 0 && digit_optind != this_option_optind)
                    printf("digits occur in two different argv-elements.\n");
                digit_optind = this_option_optind;
                printf("option %c\n", c);
                break;

            case 'a':
                printf("option a\n");
                break;

            case 'b':
                printf("option b\n");
                break;

            case 'c':
                printf("option c with value '%s'\n", optarg);
                break;

            case 'd':
                printf("option d with value '%s'\n", optarg);
                break;

            case '?':
                break;

            default:
                printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }

    return 0;
}


void do_cmd(char *cmd)
{
    int space = ' ';
    char *p[10];
    int i = 0;
    int len = strlen(cmd);

    /* 执行函数名 */
    p[i++] = "do_cmd";

    /* 参数 */
    p[i++] = cmd;

    char *pstart = cmd;
    
    /* 与发送端约定空格是参数的分割符号
     * 解析参数
     * */
    while (1) {
        pstart = strchr(pstart, space);
        if (!pstart) {
            break;
        }
        *pstart = 0;
        pstart++;

        /* 越界判断 */
        if (pstart < cmd + len) {
            p[i++] = pstart;
        }
    }

    for(int k=0; k<i; k++) {
        printf("argv[%d] %s\n", k, p[k]);
    }

    parse_cmd(i, p);
}


//void* cmdLinux(void* param)
int main(int argc, char **argv)
{
    int ret;
    int sockfd = -1, newfd;
    struct sockaddr_in seraddr;
    struct sockaddr_in clientaddr;
    socklen_t len;
    char cmd[2048];

    struct iovec vmsg[2];

    //prctl(PR_SET_NAME, "gp_cmd_server");
    //pthread_detach(pthread_self());

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("fail to creat receive cmd socket\n");
    }
    
    len = sizeof(struct sockaddr_in);
    bzero(&seraddr, len);
    seraddr.sin_family = AF_INET;
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    seraddr.sin_port = htons(1024);

    struct ifreq interface;
    strncpy(interface.ifr_ifrn.ifrn_name, INTERFAXENAME, sizeof(INTERFAXENAME));
    ret = setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (char*)&interface, sizeof(interface));
    if (ret < 0) {
        printf("setsockopt error\n");
        close(sockfd);
        return 0;
    }
    
    ret = bind(sockfd, (struct sockaddr*)&seraddr, len);
    if (ret < 0) {
        printf("bind error\n");
        close(sockfd);
        return 0;
    }

    ret = listen(sockfd, 10);
    if (ret < 0) {
        printf("listen error\n");
        close(sockfd);
        return 0;
    }
   
    while(1) {
        newfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
        if (newfd < 0) {
            printf("continue\n");
            continue;
        }
        while(1) {
            int payload_len;
            vmsg[0].iov_base = &payload_len;    
            vmsg[0].iov_len = sizeof(int);
            readv(newfd, &vmsg[0], 1);
            printf("recv payload_len %d\n", payload_len);

            memset(cmd, 0, sizeof(cmd));
            vmsg[1].iov_base = cmd;
            vmsg[1].iov_len = payload_len;
            readv(newfd, &vmsg[1], 1);

            cmd[payload_len] = 0;
            printf("recv cmd %s\n", cmd);
            log_pos = 0;
            memset(logbuf, 0, sizeof(logbuf));
            do_cmd(cmd);

            vmsg[0].iov_base = &log_pos;
            vmsg[0].iov_len = sizeof(int);
            vmsg[1].iov_base = logbuf;
            vmsg[1].iov_len = log_pos;
            writev(newfd, vmsg, 2);
            printf("write log_pos %d\n", log_pos);
            break;
        }
    }
    printf("exit cmd pthread\n");
}

