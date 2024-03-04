#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h> 
#include <sys/time.h>
#define UNIX_DOMAIN "/tmp/UNIX.domain"

//using std::cout;

typedef struct _sockMsg{
    uint16_t magic;
    uint16_t id;
    char data[8];
} sockMsg;

void on_server_data(char *buf, int len)
{
    printf("recv: %s\n", buf);
}
int socket_server_start(void)
{
#define CLIENT_NUM 5
    socklen_t clt_addr_len;
    int listen_fd;
    int clientfd[CLIENT_NUM];
    int ret;
    static char recv_buf[1024]; 
    socklen_t len;
    struct sockaddr_un clt_addr;
    struct sockaddr_un srv_addr;
    struct timeval tv;
    fd_set rfds, wfds;
    int i;
    int cnt = 0;
    int maxfd = 0;

    listen_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(listen_fd < 0)
    {
        perror("cannot create communication socket");
        return 1;
    }  
    
    srv_addr.sun_family = AF_UNIX;
    snprintf(srv_addr.sun_path, sizeof(srv_addr.sun_path), UNIX_DOMAIN);
    ret = bind(listen_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));
    if(ret == -1)
    {
        perror("cannot bind server socket");
        close(listen_fd);
        unlink(UNIX_DOMAIN);
        return 1;
    }
    ret = listen(listen_fd, CLIENT_NUM);
    if(ret == -1)
    {
        perror("cannot listen the client connect request");
        close(listen_fd);
        unlink(UNIX_DOMAIN);
        return 1;
    }

    while (1) {
        FD_ZERO(&rfds);
        FD_SET(listen_fd, &rfds);
        maxfd = listen_fd;
        for(i=0; i<cnt; i++) {
            if (clientfd[i] < 0) {
                continue;
            }
            FD_SET(clientfd[i], &rfds);
            maxfd = maxfd > clientfd[i] ? maxfd : clientfd[i];
        }
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        //printf("select:\n");
        ret = select(maxfd + 1, &rfds, NULL, NULL, &tv);
        //printf("select ret %d\n", ret);
        if (ret == -1) {
            perror("select error:");
            return 1;
        } else if (ret) {
            printf("accept come\n");
            if (FD_ISSET(listen_fd, &rfds)) {
                len = sizeof(clt_addr);
                clientfd[cnt] = accept(listen_fd, (struct sockaddr*)&clt_addr, &len);
                if(clientfd[cnt] < 0){
                    perror("cannot accept client connect request");
                    close(listen_fd);
                    unlink(UNIX_DOMAIN);
                    break;
                }
                cnt++;
                printf("accept cnt %d\n", cnt);
                //FD_SET(clientfd[cnt], &rfds);
                //FD_SET(listen_fd, &rfds);
                //FD_SET(listen_fd, &wfds);
                //FD_CLR(listen_fd, &rfds);
                continue;
            }
            for (i=0; i<cnt; i++) {
                if (!FD_ISSET(clientfd[i], &rfds)) {
                    continue;
                }
                memset(recv_buf,0,1024);
                ret = read(clientfd[i], recv_buf, sizeof(recv_buf));
                if (ret > 0) {
                    on_server_data(recv_buf, ret);
                } else if (!ret || ((ret < 0) && (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK))) {
                    printf ("socket close\n");
                    //FD_CLR(clientfd[i], &rfds);
                    close(clientfd[i]);
                    clientfd[i] = -1;
                } else {
                    printf ("socket error\n");
                    continue;
                }
            }
        } else {
            //printf("select timeout\n");
        }
    }
    
    for (i=0; i<cnt; i++) {
        if (clientfd[i] > 0)
            close(clientfd[i]);
    }
    close(listen_fd);

    unlink(UNIX_DOMAIN);
    return 0;
}

int main(void)
{

    socket_server_start();
}

