#include<stdio.h>
#include<stdint.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

void send_speed_algo(uint8_t *speed, int len)
{
//#define SPEED_FIFO "/mnt/obb/fifo"
#define SPEED_FIFO "/tmp/fifo"
    int ret;
    static int fd = 0; 
    char buf[32];
    fd_set wfds;
    struct timeval tv;
    int retval;

    if (access(SPEED_FIFO, F_OK)) {
        ret = mkfifo(SPEED_FIFO, 0644);
        if (ret == -1) {
            perror("mkfifo");
            return ;
        }
    }
    if (!fd) {
        fd = open(SPEED_FIFO, O_WRONLY | O_NONBLOCK); 
        if (fd == -1) {
            perror("fifo open");
            return;
        }
    }

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&wfds);
    FD_SET(fd, &wfds);

    /* Wait up to five seconds. */
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    retval = select(fd+1, NULL, &wfds, NULL, &tv);
    /* Don't rely on the value of tv now! */

    if (retval == -1)
        perror("select()");
    else if (retval) {
        if (FD_ISSET(0, &wfds)) {
            ret = write(fd, speed, len);  /*阻塞把数据写入有名管道*/
            printf("write fifo ret = %d", ret);
        }
    } else
        printf("No data within five seconds.\n");


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


