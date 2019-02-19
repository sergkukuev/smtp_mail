#include "sockets.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>

struct my_node* sockets_init(void)
{
	char* port = "2525";	// sockets port
    struct addrinfo* hai;   // host address info pointer
    struct addrinfo sai;    // sockets address info

    memset(&sai, 0, sizeof(sai));
    sai.ai_family = PF_UNSPEC;	// undefined type: ipv4 or ipv6
    sai.ai_socktype = SOCK_STREAM;
    sai.ai_flags = AI_PASSIVE;	// network address no specified

    if (getaddrinfo(NULL, port, &sai, &hai) != 0) {
        perror("getaddrinfo() failed");
        return NULL;
    }
    
    struct my_list* item = NULL;
    for (struct addrinfo* i = hai; i != NULL; i = i->ai_next) {
    	void* addr = &((struct sockaddr_in*)i->ai_addr)->sin_addr;
    	char ip_str[INET_ADDRSTRLEN];

    	// get readable ip 
    	inet_top(i->ai_family, addr, ip_str, sizeof(ip_str));
    	
    	// create file descriptor of socket
    	int s_fd;	// socket file descriptor
    	if (s_fd = socket(i->ai_family, i->ai_socktype, i->ai_protocol)) {
    		perror("socket() failed");
    		continue;
    	}

    	// set/get socket flags
       	int reuseaddr = 1;
    	if (setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr))) {
    		perror("setsockopt() failed");
    		continue;
    	}

    	// bind socket
    	if (bind(s_fd, i->ai_addr, i->ai_addrlen) < 0) {
    		perror("bind() failed");
    		continue;
    	}

    	// listen connect 
    	int backlog = 3;		// backlog size
    	if (listen(s_fd, backlog) < 0) {
    		perror("listen() failed");
    		continue;
    	}

    	// successfully creating
    	printf("socket created (%d)\n", s_fd);
    	struct my_node* s_next = malloc(sizeof(struct my_node));
    	s_next->data = malloc(sizeof(s_fd));
    	memset(s_next->data, s_fd, sizeof(s_fd));
    	s_next->next = item;
    	item = s_next;
    }
    freeaddrinfo(hai);
    return item;
}