#ifndef __SOCKETS_H__
#define __SOCKETS_H__

// node of any list
struct my_node {
    void* data;
    struct my_node* next;
};

// sockets initialization by getaddrinfo()
// returns head-pointer of list
struct my_node* sockets_init(void);

#endif