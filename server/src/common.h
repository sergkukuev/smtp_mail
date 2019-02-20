#ifndef __COMMON_H__
#define __COMMON_H__

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

#endif