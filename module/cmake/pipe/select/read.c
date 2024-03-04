#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<errno.h>

#define FIFO "/tmp/fifo"
#define LENGTH 50
int main(int argc,char **argv)
{
    int ret,fd; 
    char buf[LENGTH];

    if (access(FIFO,F_OK)) {
        ret = mkfifo(FIFO,0644);
        if (ret == -1) {
            perror("mkfifo");
            exit(1);
        }
        printf("create fifo\n");
    }

    printf("open fifo\n");
    fd = open(FIFO, O_RDONLY | O_NONBLOCK);
    if (fd  == -1) {
        perror("open");
        exit(1);
    }
    printf("read:\n");
    while (1) {
        memset(buf, 0, sizeof(buf));
        ret = read(fd, buf, sizeof(buf));
        if (ret <= 0) {
            perror("read error: ");
            sleep(1);
            continue;
        } else {
            printf("read ret = %d",ret);
            printf("%s",buf);
            if (strncmp(buf,"q",1) == 0) {
                break;
            }
        }
    }

    close(fd);
    return 0;
}

