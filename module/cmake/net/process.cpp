
#include <dlfcn.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <assert.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>

#include <arpa/inet.h>
#include <linux/if_arp.h>

void bond_net_device(int sock)
{
	int ret;
	struct ifreq interface;
	//const char *inf = "eth0";
	const char *inf = g_configini.netdev_name;
	
	memset(&interface, 0x00, sizeof(interface));
	strncpy(interface.ifr_name, inf, IFNAMSIZ);
	if(setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, (char *)&interface, sizeof(interface)) < 0)
	{
		perror("setsockopt error:");
        return;
	}
}

static int socket_init()
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
    
    return sock;
}

int try_connect(int sock)
{
    int ret=0;
    struct sockaddr_in host_serv_addr;
    //const char *server_ip = "192.168.100.100";
    const char *server_ip = g_configini.serverip;

    memset(&host_serv_addr, 0, sizeof(host_serv_addr));
    host_serv_addr.sin_family = AF_INET;
    //host_serv_addr.sin_port   = MY_HTONS(HOST_SERVER_PORT);
    host_serv_addr.sin_port   = MY_HTONS(g_configini.serverport);

    ret = inet_aton(server_ip, &host_serv_addr.sin_addr);
    if (0 == ret) {
        printf("inet_aton failed %d %s\n", ret, strerror(errno));
        return -1;
    }
    bond_net_device(sock);
    ret = connect(sock, (struct sockaddr *)&host_serv_addr, sizeof(host_serv_addr));
    if(ret){
        perror("connect:");
    }
    return ret;
}

#if 0
/*******************************************************************/
/* reads 'count' bytes from a socket  */
/********************************************************************/

int
Nread(int fd, char *buf, size_t count, int prot)
{
    register ssize_t r;
    register size_t nleft = count;

    while (nleft > 0) {
        r = read(fd, buf, nleft);
        if (r < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
                return NET_HARDERROR;
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

int
Nwrite(int fd, const char *buf, size_t count, int prot)
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
		return NET_SOFTERROR;

		default:
		return NET_HARDERROR;
	    }
	} else if (r == 0)
	    return NET_SOFTERROR;
	nleft -= r;
	buf += r;
    }
    return count;
}
#endif


void tcp_socket_close()
{
    printf("tcp socket close!\n");
    close(hostsock);
}

void *pthread_tcp_recv(void *para)
{
    int32_t ret = 0;
    int i=0;
    static int tcprecvcnt = 0;
    uint8_t *readbuf = NULL;
    uint8_t *msgbuf = NULL;

    prctl(PR_SET_NAME, "tcp_process");
    send_stat_pkg_init();

    msgbuf = (uint8_t *)malloc(RECV_HOST_DATA_BUF_SIZE);
    if(!msgbuf)
    {
        perror("parse_host_cmd malloc");
        return NULL;
    }
    readbuf = (uint8_t *)malloc(TCP_READ_BUF_SIZE);
    if(!readbuf){
        perror("tcp readbuf malloc");
        goto out;
    }

connect_again:
    hostsock = socket_init();
    if(hostsock < 0){
        goto out;
    }
    while (!force_exit) {
        if(try_connect(hostsock)){
            sleep(1);
            printf("try connect!\n");
            continue;
        }else{
            printf("connected!\n");
        }
#if defined ENABLE_ADAS
        send_work_status(SAMPLE_DEVICE_ID_ADAS);
#elif defined ENABLE_DMS
        send_work_status(SAMPLE_DEVICE_ID_DMS);
#endif

        while(1){
#if 1
            ret = read(hostsock, readbuf, TCP_READ_BUF_SIZE);
            if (ret < 0) {
                printf("read failed %d %s\n", ret, strerror(errno));
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK){
                    usleep(10000);
                    continue;
                }else{
                    //break;
                    goto connect_again;
                }
            }else if (ret == 0) {
                close(hostsock);
                printf("tcp disconnect! sock = %d\n",hostsock);
                //hostsock = -1;
                goto connect_again;
                //break;
            }else{//write to buf
                //MY_DEBUG("recv raw cmd, tcprecvcnt = %d:\n", tcprecvcnt++);
                //printbuf(readbuf, ret);
                i=0;
                while(ret--){
                    parse_cmd(&readbuf[i++], msgbuf);
                }
            }
#else
            fd_set rfds;
            struct timeval tv;
            int retval;
            FD_ZERO(&rfds);
            FD_SET(hostsock, &rfds);

            /* Wait up to five seconds. */
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            retval = select(hostsock+1, &rfds, NULL, NULL, &tv);
            if (retval == -1 ){
                if(errno != EINTR){
                    perror("select()");
                    close(hostsock);
                    hostsock = -1;
                    goto connect_again;
                }
            }else if (retval){
                if(FD_ISSET(hostsock, &rfds)){
                    retval = read(hostsock, readbuf, TCP_READ_BUF_SIZE);
                    if (retval < 0) {
                        printf("read failed %d %s\n", retval, strerror(errno));
                        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK){
                            usleep(10000);
                        }else if (retval == 0) {
                            close(hostsock);
                            hostsock = -1;
                            goto connect_again;
                        }else{
                            i=0;
                            while(ret--){
                                parse_cmd(&readbuf[i++], msgbuf);
                            }
                        }
                    }
                }
            }else{
                printf("No data within five seconds.\n");
            }

#endif
        }
    }
out:
    printf("%s exit!\n", __FUNCTION__);
    if(readbuf)
        free(readbuf);
    if(msgbuf)
        free(msgbuf);

    if(hostsock>0)
        close(hostsock);
    pthread_exit(NULL);
}
