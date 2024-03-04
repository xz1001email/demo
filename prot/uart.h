#ifndef  __UART_IO_H__
#define __UART_IO_H__

#include <stdint.h>
#include "prot.h"

int uart_close(prot_handle *handle);
int uart_init(prot_handle *handle);
int uart_snd(prot_handle *handle);
int uart_rcv(prot_handle *handle);

#endif

