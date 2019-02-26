#ifndef __SOCKETS_H__
#define __SOCKETS_H__

#include "common.h"

// defines reply and receive
#define DATA_FAILED -2
#define DATA_BLOCK -1
#define DATA_EMPTY 0

// initialize listen socket
// returns fd
int init_listen_socket();

// close listen socket
int close_listen_socket(int fd);

// initialize client socket
// returns data of socket
struct cs_data_t* bind_client_data(int fd, struct sockaddr addr, int state);

// free data of client
void free_client_data(struct cs_data_t** data);

// close client socket
// returns node_cs->next
struct cs_node_t* close_client_socket(struct cs_node_t* node_cs);

// close clients socket by state
// state = SOCKET_NOSTATE - close all sockets in list
// returns count of sockets
int close_client_sockets_by_state(struct cs_node_t** head_cs, int state);

// accept client socket
// return client data
struct cs_data_t* accept_client_socket(int fd);

// sending data
int send_data(int fd, char* bf, size_t bfsz, int flags);

// receive data
int recv_data(int fd, char* bf, size_t bfsz, int flags);

#endif