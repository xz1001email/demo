#ifndef  __NET_IO_H__
#define __NET_IO_H__

#include <stdint.h>
#include "prot.h"

int socket_close(prot_handle *handle);
int socket_init(prot_handle *handle);
int try_connect(prot_handle *handle);
int tcp_snd(prot_handle *handle);
int tcp_rcv(prot_handle *handle);

#endif
