#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "common.h"

// struct of process data
struct process_t {
    pid_t pid;
    pid_t lgpid;
    bool worked;

    struct fd_t {
        int listen;
        int logger;
        int exit;
        int max;
    } fd;

    // sets
    fd_set writefds;
    fd_set readfds;
    // lists of clients sockets
    struct cs_node_t* ws_list;
    struct cs_node_t* rs_list;
};

// create and run process
// returns pid
pid_t create_process(int* listen_fd, pid_t* log_pid, void (*body)(int*, pid_t*));

// create and run processes
// returns pids array
pid_t* create_processes(int s_fd, int* nproc, pid_t log_pid);

// process initialization
// return structure
struct process_t* init_process(int* fd, pid_t log_pid);

// process free
void free_process(struct process_t* proc);

#endif