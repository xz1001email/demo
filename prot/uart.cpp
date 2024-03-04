
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>
#include<termios.h>
#include<string.h>
#include <pthread.h>
#include <sys/time.h>
#include <linux/serial.h>

#include <sys/ioctl.h>

#define TIOCSRS485	0x542F

#include "uart.h"

int config_rs485(int fd)
{
    struct serial_rs485 rs485conf;
    memset(&rs485conf, 0, sizeof(rs485conf));
	rs485conf.flags |= SER_RS485_ENABLED;    // enable 485
	// Set logical level for RTS pin equal to 1 when sending
    rs485conf.flags |= SER_RS485_RTS_ON_SEND;
    // rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;
	if (ioctl(fd, TIOCSRS485, &rs485conf) < 0) {
		perror("ioctl TIOCSRS485 error");
        return -1;
	}
    
    return 0;
}

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop)
{
    struct termios newtio,oldtio;
    if( tcgetattr(fd, &oldtio)  !=  0) {
        perror("tcgetattr error");
        return -1;
    }
    bzero( &newtio, sizeof( newtio ) );
    newtio.c_cflag  |=  CLOCAL | CREAD; 
    newtio.c_cflag &= ~CSIZE; 

    switch( nBits )
    {
        case 7:
            newtio.c_cflag |= CS7;
            break;
        case 8:
            newtio.c_cflag |= CS8;
            break;
    }

    switch( nEvent )
    {
        case 'O':
        case 'o':
            newtio.c_cflag |= PARENB; 
            newtio.c_cflag |= PARODD;  
            newtio.c_iflag |= (INPCK | ISTRIP); 
            break;
        case 'E':
        case 'e':
            newtio.c_iflag |= (INPCK | ISTRIP);
            newtio.c_cflag |= PARENB;
            newtio.c_cflag &= ~PARODD;
            break;
        case 'N': 
        case 'n': 
            newtio.c_cflag &= ~PARENB;
            break;
    }

    switch( nSpeed )
    {
        case 2400:
            cfsetispeed(&newtio, B2400);
            cfsetospeed(&newtio, B2400);
            break;
        case 4800:
            cfsetispeed(&newtio, B4800);
            cfsetospeed(&newtio, B4800);
            break;
        case 9600:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
        case 19200:
            cfsetispeed(&newtio, B19200);
            cfsetospeed(&newtio, B19200);
            break;
        case 38400:
            cfsetispeed(&newtio, B38400);
            cfsetospeed(&newtio, B38400);
            break;
        case 115200:
            cfsetispeed(&newtio, B115200);
            cfsetospeed(&newtio, B115200);
            break;
        case 230400:
            cfsetispeed(&newtio, B230400);
            cfsetospeed(&newtio, B230400);
            break;
        case 460800:
            cfsetispeed(&newtio, B460800);
            cfsetospeed(&newtio, B460800);
            break;
        case 921600:
            cfsetispeed(&newtio, B921600);
            cfsetospeed(&newtio, B921600);
            break;
        default:
            cfsetispeed(&newtio, B9600);
            cfsetospeed(&newtio, B9600);
            break;
    }

    if( nStop == 1){
        newtio.c_cflag &=  ~CSTOPB; 
    }else if ( nStop == 2 ){
        newtio.c_cflag |=  CSTOPB;
    } 
    newtio.c_cc[VTIME]  = 0;
    newtio.c_cc[VMIN] = 0;
    tcflush(fd, TCIFLUSH); 
    if((tcsetattr(fd, TCSANOW, &newtio))!=0)
    {
        perror("set error");
        return -1;
    }
    return 0;
}

int uart_close(prot_handle *handle)
{
    if(handle->fd > 0)
        return close(handle->fd);

    return -1;
}

int uart_init(prot_handle *handle)
{
    int fd;
    int err;

    LocalConfig *config = handle->config;
    printf("uart config:\n");
    printf("tty_name: %s\n", config->tty_name);
    printf("baudrate: %d\n", config->baudrate);
    printf("parity str: %s\n", config->parity);
    printf("parity: %c\n", config->parity[0]);
    printf("stopbit: %d\n", config->stopbit);
    printf("bits: %d\n", config->bits);

    //fd = open(config->tty_name, O_RDWR | O_NDELAY)
    fd = open(config->tty_name, O_RDWR | O_NDELAY);
    if(fd < 0){
        printf("open tty fail, error:%s\n", strerror(errno));
        return -1;
    }
    err = set_opt(fd, config->baudrate,  config->bits, config->parity[0], config->stopbit);
    if(err){
        printf("set tty fail, error:%s\n", strerror(errno));
        return -1;
    }

    handle->fd = fd;
    return 0;
}

int uart_snd(prot_handle *handle)
{
    uint8_t *buf = handle->sndData_s;
    int fd = handle->fd;
    ssize_t count = handle->sndlen;
    size_t nleft = count;
    ssize_t r;

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
int uart_rcv(prot_handle *handle)
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
                    printf("fd error:%s", strerror(errno));
                    return -1;
                }
            }else if(r == 0){ /*no data*/

            }else{ /*recv data*/
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

