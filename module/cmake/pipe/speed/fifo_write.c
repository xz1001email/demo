#include<stdio.h>
#include<stdint.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<errno.h>

#define FIFO "./FIFO" /*有名管道的名字*/
#define LENGTH 50 /*字符数组的长度*/

void send_speed_algo(uint8_t *speed, int len)
{
#define SPEED_FIFO "/mnt/obb/fifo"
    int ret;
    static int fd = 0; 
    char buf[32];
    if (access(SPEED_FIFO, F_OK)) {
        ret = mkfifo(SPEED_FIFO, 0644);
        if (ret == -1) {
            perror("mkfifo");
            return ;
        }
    }
    if (!fd) {
        fd = open(SPEED_FIFO, O_WRONLY); 
        if (fd == -1) {
            perror("fifo open");
            return;
        }
    }
    ret = write(fd, speed, len);  /*阻塞把数据写入有名管道*/
    printf("write fifo ret = %d", ret);
}

int main(int argc,char **argv)
{
    uint8_t speed = 0;

    if(argc < 2) {
        printf("input speed you need...\n");
        return -1;
    }

    speed = strtol(argv[1], NULL, 10);
    printf("set speed = %d\n");

    send_speed_algo(&speed, 1);

    return 0;
}


