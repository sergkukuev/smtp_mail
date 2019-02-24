#include "sockets.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// ERRORS
 
#define ERR_SOCKET -1
#define ERR_SOCKOPT -2
#define ERR_BIND -3
#define ERR_LISTEN -4
#define ERR_FCNTL -5
#define ERR_ACCEPT -6
#define ERR_CLOSE -7
#define ERR_GETADDRINFO -8

// parse sockets error
void parse_error(int err)
{
	switch (err) {
		case ERR_SOCKET:
			perror("socket() failed");
			break;
		case ERR_SOCKOPT:
			perror("setsockopt() failed");
			break;
		case ERR_BIND:
			perror("bind() failed");
			break;
		case ERR_LISTEN:
			perror("listen() failed");
			break;
		case ERR_FCNTL:
			perror("fcntl() failed");
			break;
		case ERR_ACCEPT:
			perror("accept() failed");
			break;
		case ERR_CLOSE:
			perror("close() failed");
			break;
		case ERR_GETADDRINFO:
			perror("getaddrinfo() failed");
			break;
		default:
			perror("undefined error");
			break;
	}
	printf("Server(%d): socket failed (%d)", getpid(), err);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// LISTEN SOCKET

// create listen connection
// return fd or error
int create_listen_socket(struct addrinfo* inst)
{
	void* addr = &((struct sockaddr_in*)inst->ai_addr)->sin_addr;
    char ip_str[INET6_ADDRSTRLEN];

	// get readable ip 
	inet_ntop(inst->ai_family, addr, ip_str, sizeof(ip_str));

	// create file descriptor of socket
	int s_fd = socket(inst->ai_family, inst->ai_socktype, inst->ai_protocol);
	if (s_fd == -1)
		return ERR_SOCKET;

	int opt = 1;
	if (setsockopt(s_fd, SOL_SOCKET, SO_REUSEPORT | SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		return ERR_SOCKOPT;

	// get flags
	int flags = fcntl(s_fd, F_GETFL, 0);
	if (flags == -1)
		return ERR_FCNTL;

	//set non block flag
	if (fcntl(s_fd, F_SETFL, flags | O_NONBLOCK) == -1)
		return ERR_FCNTL;

	if (bind(s_fd, inst->ai_addr, inst->ai_addrlen) == -1)
		return ERR_BIND;

	if (listen(s_fd, BACKLOG_SIZE) == -1)
		return ERR_LISTEN;
	return s_fd;
}

// close connection
void close_listen_socket(int fd)
{
	if (close(fd) != 0)
		parse_error(ERR_CLOSE);
}

// create listener socket by getaddrinfo()
// return fd
int init_listen_socket()
{
    struct addrinfo* hai;   // host address info pointer
    struct addrinfo sai;    // sockets address info
	// ((struct sockaddr_in*)sai.ai_addr)->sin_addr.s_addr = inet_addr(SERVER_ADDR);

	printf("Server(%d): listen socket address - %s\n", getpid(), SERVER_ADDR);
	printf("Server(%d): listen socket port - %s\n", getpid(), SERVER_PORT);

    memset(&sai, 0, sizeof(sai));
    sai.ai_family = AF_UNSPEC;	// undefined type: ipv4 or ipv6
    sai.ai_socktype = SOCK_STREAM;
    sai.ai_flags = AI_PASSIVE;	// network address no specified

    if (getaddrinfo(NULL, SERVER_PORT, &sai, &hai) != 0) {
        parse_error(ERR_GETADDRINFO);
		return ERR_GETADDRINFO;
    }

	// initialize
	int fd = create_listen_socket(hai);
	fd < 0 ? parse_error(fd) : printf("Server(%d): create listen socket (%d)\n", getpid(), fd);
	freeaddrinfo(hai);
	return fd;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// CLIENT SOCKET

// initialize client socket
// returns data of socket
struct cs_data_t* bind_client_data(int fd, int state)
{
	struct cs_data_t* data = malloc(sizeof(*data));
	data->fd = fd;
	data->state = state;
	data->msg = NULL;
	data->buf = malloc(sizeof(*(data->buf)) * BUFFER_SIZE);
	data->offset = 0;
	printf("Server(%d): bind data to socket (fd = %d)\n", getpid(), fd);
	return data;
}

// free data of client
void free_client_data(struct cs_data_t** data)
{
	struct cs_data_t* tmp = *data;
	if (tmp->buf != NULL)
		free(tmp->buf);

	if (tmp->msg != NULL) {
		if (tmp->msg->body != NULL)
			free(tmp->msg->body);
		if (tmp->msg->from != NULL)
			free(tmp->msg->from);
		for (int i = 0; i < tmp->msg->rnum; i++)
			if (tmp->msg->to[i] != NULL)
				free(tmp->msg->to[i]);
		free(tmp->msg);
	}
	free(*data);
	*data = NULL;
}

// close clients socket by state
// state = SOCKET_NOSTATE - close all sockets in list
void close_client_sockets_by_state(struct cs_node_t** head, int state)
{
	struct cs_node_t* prev = NULL;
	for (struct cs_node_t* tmp = *head; tmp != NULL; tmp = tmp->next) {
		if (tmp->cs->state == state || state == SOCKET_NOSTATE) {
			if (prev == NULL)	// move head
				*head = tmp->next;
			prev->next = tmp->next;
			if (close(tmp->cs->fd) != 0)	// close socket
				parse_error(ERR_CLOSE);
			free_client_data(&tmp->cs);
			tmp = prev->next;
		}
	}
}

// accept connections
// fd - listen file description
struct cs_data_t* accept_client_socket(int fd)
{
	// accept connection
	struct sockaddr_in addr;
	socklen_t addrlen;
	get_address(&addr, &addrlen);
	int new_fd = accept(fd, (struct sockaddr*) &addr, &addrlen);
	if (new_fd == -1) {
		parse_error(ERR_ACCEPT);
		return NULL;
	}
	// get flags
	int flags = fcntl(fd, F_GETFL, 0); 
	if (flags == -1) {
		parse_error(ERR_FCNTL);
		return NULL;
	}
	// set nonblock flag
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1); {
		parse_error(ERR_FCNTL);
		return NULL;
	}
	return bind_client_data(new_fd, SOCKET_STATE_START);
}