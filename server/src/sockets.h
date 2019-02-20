#ifndef __SOCKETS_H__
#define __SOCKETS_H__

#include "common.h"

// server sockets initialization by getaddrinfo()
// returns head-pointer of list
struct ss_node_t* init_serv_sockets(void);

// client sockets initialization
// returns head-pointer of list
struct cs_node_t* init_client_sockets(struct ss_node_t* ss_list, int* max_fd);

#endif