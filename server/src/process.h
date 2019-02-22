#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "common.h"

// struct of process data
struct process_t {
    int pid;
    bool worked;
    int l_fd;       // listen fd
    int max_fd;     // used select

    // uses as fd
    int* mq;  // used only logger
    int lg;

    // sets
    fd_set writefds;
    fd_set readfds;
    // lists of clients sockets
    struct cs_node_t* ws_list;
    struct cs_node_t* rs_list;
};

// create and run process
// returns pid
int create_process(int* listen_fd, int log_pid, void (*body)(int*, int));

// create and run processes
// returns pids array
int* create_processes(int s_fd, int log_pid, int* nproc);

// process initialization
// return structure
struct process_t* init_process(int pid, int log_pid, int* fd);

// process free
void free_process(struct process_t* proc);

#endif