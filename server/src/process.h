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
        int cmd;    // command message queue
        int max;
    } fd;

    // sets
    fd_set* writefds;
    fd_set* readfds;

    struct cs_node_t* s_list; // lists of clients sockets
};

// create and run process
// returns pid
pid_t create_process(int* listen_fd, pid_t* log_pid, void (*body)(int*, pid_t*));

// create and run processes
// returns numeric of creating processes
int create_processes(int s_fd, pid_t* procs, int nproc, pid_t log_pid);

// process initialization
// return structure
struct process_t* init_process(int* fd, pid_t log_pid);

// process free
void free_process(struct process_t* proc);

#endif