#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>
#include <linux/if_arp.h>

/* According to POSIX.1-2001, POSIX.1-2008 */
#include <sys/select.h>

/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#include "net.h"

void bond_net_device(int fd, char *device)
{
	int ret;
	struct ifreq interface;
	//const char *inf = "eth0";
	
	memset(&interface, 0x00, sizeof(interface));
	strncpy(interface.ifr_name, device, IFNAMSIZ);
	if(setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&interface, sizeof(interface)) < 0)
	{
		perror("bind device error:");
        return;
	}
}
int socketkeepalive(int fd)
{
    int keepAlive=1;//开启keepalive属性
    int keepIdle=90;//在此时间没有数据则进行探测。
    int keepInterval=2;//探测时发包的时间间隔为2秒
    int keepCount=3;//探测尝试的次数。如果第1次探测包就收到响应了，则后2次的不再发送
    if(setsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(void *)&keepAlive,sizeof(keepAlive))!=0){
		perror("set TCP_KEEPALIVE error:");
        return -1;
    }
    keepAlive=0;
    socklen_t len=0;
    if(getsockopt(fd,SOL_SOCKET,SO_KEEPALIVE,(void *)&keepAlive,&len)!=0){
		perror("set TCP_KEEPALIVE error:");
        return -1;
    }
    printf("get keepAlive = %d\n", keepAlive);
    if(setsockopt(fd,SOL_TCP,TCP_KEEPIDLE,(void *)&keepIdle,sizeof(keepIdle))!=0){
		perror("set TCP_KEEPIDLE error:");
        return -1;
    }
    if(setsockopt(fd,SOL_TCP,TCP_KEEPINTVL,(void *)&keepInterval,sizeof(keepInterval))!=0){
		perror("set TCP_KEEPINTVAL error:");
        return -1;
    }
    if(setsockopt(fd,SOL_TCP,TCP_KEEPCNT,(void *)&keepCount,sizeof(keepCount))!=0){
		perror("set TCP_KEEPCNT error:");
        return -1;
    }
    return 0;
}

int try_connect(prot_handle *handle)
{

    int fd = handle->fd;
    LocalConfig *config = handle->config;

    int ret=0;
    struct sockaddr_in host_serv_addr;
    //const char *server_ip = "192.168.100.100";
    const char *server_ip = config->serverip;

    memset(&host_serv_addr, 0, sizeof(host_serv_addr));
    host_serv_addr.sin_family = AF_INET;
    //host_serv_addr.sin_port   = MY_HTONS(HOST_SERVER_PORT);
    host_serv_addr.sin_port   = MY_HTONS(config->serverport);

    ret = inet_aton(server_ip, &host_serv_addr.sin_addr);
    if (0 == ret) {
        printf("inet_aton failed %d %s\n", ret, strerror(errno));
        return -1;
    }
    printf("try connect!\n");
    ret = connect(fd, (struct sockaddr *)&host_serv_addr, sizeof(host_serv_addr));
    if(ret){
        perror("connect error:");
    }
    return ret;
}

int socket_init(prot_handle *handle)
{
#define HOST_SERVER_PORT (8888)
    int sock;
    int32_t ret = 0;
    int enable = 1;
    socklen_t optlen;
    int bufsize = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Create socket failed %s\n", strerror(errno));
        return -1;
    }

    handle->fd = sock;
    LocalConfig *config = handle->config;

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable));

    bufsize = 0;
    optlen = sizeof(bufsize);
    getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, &optlen);
    printf("get recv buf size = %d\n", bufsize);
    getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, &optlen);
    printf("get send buf size = %d\n", bufsize);
    //int setsockopt(int sockfd, int level, int optname,const void *optval, socklen_t optlen);

    printf("set buf size = %d\n", bufsize);
    bufsize = 64*1024;
    optlen = sizeof(bufsize);
    ret = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, optlen);
    if(ret == -1)
    {
        printf("%s:%d error\n", __FILE__, __LINE__);
        return -1;
    }
    bufsize = 64*1024;
    optlen = sizeof(bufsize);
    ret = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, optlen);
    if(ret == -1)
    {
        printf("%s:%d error\n", __FILE__, __LINE__);
        return -1;
    }

    bufsize = 0;
    optlen = sizeof(bufsize);
    getsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, &optlen);
    printf("get recv buf size = %d\n", bufsize);
    getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, &optlen);
    printf("get send buf size = %d\n", bufsize);
    
    bond_net_device(sock, config->netdev_name);
    //socketkeepalive(sock);

    return sock;
}

/*******************************************************************/
/* reads 'count' bytes from a socket  */
/********************************************************************/

int Nread(int fd, char *buf, size_t count, int prot)
{
    register ssize_t r;
    register size_t nleft = count;

    while (nleft > 0) {
        r = read(fd, buf, nleft);
        if (r < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
                return -1;
        } else if (r == 0)
            break;

        nleft -= r;
        buf += r;
    }
    return count - nleft;
}


/*
 *                      N W R I T E
 */

int Nwrite(int fd, const char *buf, size_t count, int prot)
{
    register ssize_t r;
    register size_t nleft = count;

    while (nleft > 0) {
        r = write(fd, buf, nleft);
        if (r < 0) {
            switch (errno) {
                case EINTR:
                case EAGAIN:
#if (EAGAIN != EWOULDBLOCK)
                case EWOULDBLOCK:
#endif
                    return count - nleft;

                case ENOBUFS:
                    return -1;

                default:
                    return -1;
            }
        } else if (r == 0)
            return -2;
        nleft -= r;
        buf += r;
    }
    return count;
}
//return if error happened, or data write over
int tcp_snd(prot_handle *handle)
{
    uint8_t *buf = handle->sndData_s;
    int  len = handle->sndlen;
    int fd = handle->fd;

    int ret = 0;
    int offset = 0;
    
    //printf("sendlen = %d\n", len);

    if(fd< 0 || len < 0 || !buf){
        return -1;
    } else{
        while(offset < len){
            ret = write(fd, &buf[offset], len-offset);
            if(ret <= 0){
                //write error deal
                perror("tcp write:");
                if(errno == EINTR || errno == EAGAIN){
                    printf("package write wait mommemt, continue!\n");
                    //usleep(10000);
                    continue;
                }else
                    return -1;
            }else{
                offset += ret;
            }
        }
    }
    return 0;
}
#if 0
int do_recv_msg(int fd, uint8_t *buf, int count, uint8_t *msg)
{
    ssize_t r;
    int i=0;

    r = read(fd, buf, count);
    if (r < 0) {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            return 0;
        else
            return -1;
    } else if (r == 0)
            return -1;

    while(r--){
        prot_parse(&buf[i++], msg);
    }

    return 0;
}
#endif

void printbuf(void *buffer, int len);
int tcp_rcv(prot_handle *handle)
{
    int fd = handle->fd;
    struct timeval tv;
    int retval;
    ssize_t r;
    int i=0;

    uint8_t *buf = handle->rcvData_s;
    uint8_t *msg = handle->rcvData;

    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    retval = select(fd+1, &rfds, NULL, NULL, &tv);
    if (retval == -1 && errno != EINTR){ //error
        perror("select()");
        return -1;
    }
    if(retval > 0){
        if(FD_ISSET(fd, &rfds)){
            r = read(fd, buf, SLIP_DATA_BUF_SIZE);
            if(r < 0) {
                if(errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK){
                    return 0;
                }else{ /*error*/
                    printf("socket error and connect ..., error:%s", strerror(errno));
                    handle->close(handle);
                    return -1;
                }
            }else if(r == 0){ /*close*/
                printf("socket close and connect ..., error:%s", strerror(errno));
                handle->close(handle);
                return -1;
            }else{ /*recv data*/
                //printf("----------------------recv:");
                //printbuf(buf, r);
                handle->rcvlen = r;
                handle->parse(handle);
            }
        }
        if(retval== 0){
        }
        return 0;
    }

    return 0;
}

int socket_close(prot_handle *handle)
{
    if(handle->fd > 0)
        return close(handle->fd);

    return -1;
}


