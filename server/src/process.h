#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "common.h"

// create and run process
// returns process structure
struct process_t* create_process(struct ss_node_t* socket_fd, pid_t log_pid);

#endif