#ifndef __COMMON_H__
#define __COMMON_H__

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <mqueue.h>

// node of server socket
struct ss_node_t {
    int fd;     // file descriptor
    struct fd_note* next;
};

// node of client socket
struct cs_node_t {
    // TODO: some data
    struct cs_node_t* next;
};

// struct of process data
struct process_t {
    pid_t pid;
    // message queue params
    mqd_t* mq;
    char* mq_name;
    fd_set ss_set;

    struct ss_node_t* ss;
};

#endif