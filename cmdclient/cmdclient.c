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

int main(int argc, char* argv[])
{
    int ret;
    char cmd[2048];
    struct iovec vmsg[2];
    int payload_len = 0;
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    if (argc <= 1) {
        printf("try --help.\n");
        return -1;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        printf("create client sock error\n");
        return 0;
    }

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(1024);
    ret = connect(fd, (struct sockaddr*)&addr, len);
    if (ret < 0) {
        printf("client connect error:%s\n", strerror(errno));
        close(fd);
        return ret;
    }

    memset(cmd, 0, sizeof(cmd));
    for (int i = 1; i < argc; i++) {
        strcat(cmd, argv[i]);
        strcat(cmd, " ");
    }

    payload_len = strlen(cmd);
    vmsg[0].iov_base = &payload_len;    
    vmsg[0].iov_len = sizeof(int);
    vmsg[1].iov_base = cmd;
    vmsg[1].iov_len = payload_len;
    writev(fd, &vmsg[0], 2);

    vmsg[0].iov_base = &payload_len;    
    vmsg[0].iov_len = sizeof(int);
    readv(fd, &vmsg[0], 1);
    //printf("payload_len %d\n", payload_len);
    
    memset(cmd, 0, sizeof(cmd));
    vmsg[1].iov_base = cmd;
    vmsg[1].iov_len = payload_len;
    readv(fd, &vmsg[1], 1);

    cmd[payload_len] = 0;
    printf("%s", cmd);

    close(fd);
    return 1;
}

