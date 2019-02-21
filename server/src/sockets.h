#ifndef __SOCKETS_H__
#define __SOCKETS_H__

#include "common.h"

// server sockets initialization by getaddrinfo()
// returns head-pointer of list
struct ss_node_t* init_serv_sockets();

// client sockets initialization
// returns head-pointer of list
struct cs_node_t* init_client_sockets(struct ss_node_t* ss_list, int* max_fd);

// delete all clients sockets by state
// returns new head header
struct cs_node_t* delete_sockets_by_state(struct cs_node_t* cs, int state);

// delete all sockets
// return numeric sockets
int delete_client_sockets(struct cs_node_t** cs);

// parse result function select() inside process
void parse_select(struct process_t* proc);

#endif