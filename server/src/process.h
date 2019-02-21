#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "common.h"

// create and run process
// returns process structure
pid_t create_process(struct ss_node_t* socket_fd, pid_t log_pid);

// process initialization (for logger)
struct process_t* init_process(pid_t pid, pid_t log_pid, struct ss_node_t* ss);

#endif