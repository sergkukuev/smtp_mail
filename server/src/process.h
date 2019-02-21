#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "common.h"

// create and run process
// returns process id
pid_t create_process(struct ss_node_t* socket_fd);

// for debug in one process
void body_process(struct ss_node_t* fd_sock);

#endif