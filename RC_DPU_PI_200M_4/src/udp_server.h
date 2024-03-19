/*
 * udp_server.h
 *
 *  Created on: 2024. 1. 24.
 *      Author: Lee Jeho
 */

#ifndef SRC_APP_NETWORKING_UDP_SERVER_H_
#define SRC_APP_NETWORKING_UDP_SERVER_H_

#include <stdio.h>
#include "xparameters.h"
#include "platform.h"
#include "platform_config.h"

#include "netif/xadapter.h"
#include "lwipopts.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/init.h"

#include "sleep.h"

#define UDP_SEND_BUFSIZE 1440U

extern struct netif server_netif;

struct netif *UdpInitUser(void);
int TransferData(void);

#endif /* SRC_APP_NETWORKING_UDP_SERVER_H_ */
