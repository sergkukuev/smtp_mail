#ifndef __SOCKETS_H__
#define __SOCKETS_H__

#include "common.h"

// initialize listen socket
// returns fd
int init_listen_socket();

// close listen socket
void close_listen_socket(int fd);

// bind data to a socket
// returns data of socket
struct cs_data_t* bind_client_data(int fd, int state);

// free data of client
void free_client_data(struct cs_data_t** data);

// close client socket
// returns node_cs->next
struct cs_node_t* close_client_socket(struct cs_node_t* node_cs);

// close clients socket by state
// state = SOCKET_NOSTATE - close all sockets in list
void close_client_sockets_by_state(struct cs_node_t** head_cs, int state);

// accept client socket
// return client data
struct cs_data_t* accept_client_socket(int fd);

#endif