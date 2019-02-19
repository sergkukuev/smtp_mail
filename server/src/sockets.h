#ifndef __SOCKETS_H__
#define __SOCKETS_H__

// node of file descriptor
struct fd_node {
    int fd;
    struct fd_note* next;
};

// server sockets initialization by getaddrinfo()
// returns head-pointer of list
struct fd_node* init_serv_sockets(void);

#endif