#include<stdio.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<errno.h>

#define FIFO "./FIFO" /*有名管道的名字*/
#define LENGTH 50 /*字符数组的长度*/
int main(int argc,char **argv)
{
    int ret, fd = -1;
    char buf[LENGTH];

    if (access(FIFO,F_OK)) {
        ret = mkfifo(FIFO,0644);
        if (ret == -1) {
            perror("mkfifo");
            exit(1);
        }
    }

    puts("请输入数据到有名管道中,输入q退出：");
    while(1)
    {
        if (fd == -1) {
            fd = open(FIFO, O_WRONLY | O_NONBLOCK);
            if (fd == -1) {
                perror("open");
                sleep(1);
                continue;
            } else if (fd > 0) {
                printf("write open ok!\n");
            }
        }

        memset(buf,0,sizeof(buf)); /*清空缓冲区*/
        fgets(buf,sizeof(buf),stdin); /*从终端输入数据到buf中*/
        write(fd,buf,strlen(buf));  /*阻塞把数据写入有名管道*/

        if (strncmp(buf,"q",1) == 0) {
            printf("aaaa\n");
            break;
        }
    }

    close(fd); /*关闭有名管道*/
    return 0;

}
