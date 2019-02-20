#ifndef __COMMON_H__
#define __COMMON_H__

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <mqueue.h>

// server data
#define BUFFER_SIZE 1024
#define BACKLOG_SIZE 3

// socket state
#define SOCKET_STATE_INIT 0
#define SOCKET_STATE_WAIT 1
#define SOCKET_STATE_CLOSED 2

//struct of message
struct msg_t {
    char** to;
    char* from;
    char* body;
    int blen;   // body length
    int rnum;   // recepients numeric
};

// node of server socket
struct ss_node_t {
    int fd;     // file descriptor
    struct ss_node_t* next;
};

// data of client socket
struct cs_data_t {
    int fd;
    int state;
    bool flag;     // false - read set fd, true - write set fd
    struct msg_t* message;
};

// node of client socket
struct cs_node_t {
    struct cs_data_t cs;
    struct cs_node_t* next;
};

// struct of process data
struct process_t {
    pid_t pid;
    bool worked;
    int max_fd;

    // sets
    fd_set writefds;
    fd_set readfds;

    // message queue params
    mqd_t* mq;
    char* mq_name;

    // lists of clients sockets
    struct cs_node_t* ss_list;
    struct cs_node_t* ls_list;
};

#endif