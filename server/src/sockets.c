#include "sockets.h"
#include "smtp_def.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// SEND & RECEIVE

// sending data
int send_data(int fd, char* bf, size_t bfsz, int flags)
{
    if (fd <= 0)    return DATA_FAILED;
    int nbytes = send(fd, bf, bfsz, flags);
    return nbytes < 0 ? ((errno == EWOULDBLOCK) ? DATA_BLOCK : DATA_FAILED) : nbytes;
}

// recv data
int recv_data(int fd, char* bf, size_t bfsz, int flags)
{
    if (fd <= 0)    return DATA_FAILED;
    int nbytes = recv(fd, bf, bfsz, flags);
    return nbytes < 0 ? ((errno == EWOULDBLOCK) ? DATA_BLOCK : DATA_FAILED) : nbytes;
}

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
	//printf("Worker(%d): socket failed (%d)\n", getpid(), err);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// LISTEN SOCKET

// create listen connection
// return fd or error
int create_listen_socket(struct addrinfo* inst)
{
	void* addr;
	char ip_str[INET6_ADDRSTRLEN];

	if (inst->ai_family == AF_INET)
		addr = &((struct sockaddr_in*)inst->ai_addr)->sin_addr;
    else
		addr = &((struct sockaddr_in6*)inst->ai_addr)->sin6_addr;

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
int close_listen_socket(int fd)
{
	int result = close(fd);
	if (result != 0)
		parse_error(ERR_CLOSE);
	return result;
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

void free_message_struct(struct msg_t** msg)
{
	if ((*msg)->body != NULL) {
		free((*msg)->body);
		(*msg)->blen = 0;
	}
	if ((*msg)->from != NULL) 
		free((*msg)->from);
	
	if ((*msg)->to != NULL) {
		for (int i = 0; i < MAX_RECIPIENTS; i++)
			if ((*msg)->to[i] != NULL)
				free((*msg)->to[i]);
			free((*msg)->to);
			(*msg)->rnum = 0;
	}
	free(*msg);
	*msg = NULL;
}

void init_message_struct(struct msg_t** msg)
{
	if (*msg != NULL)
		free_message_struct(msg);
	
	int mailcount = 150;
	*msg = (struct msg_t*) malloc(sizeof(struct msg_t));
	(*msg)->from = (char*) malloc(sizeof(char) * mailcount);
	(*msg)->to = (char**) malloc(sizeof(char*) * MAX_RECIPIENTS);
	for (int i = 0; i < MAX_RECIPIENTS; i++)
		(*msg)->to[i] = (char*)malloc(sizeof(char) * mailcount);
	(*msg)->rnum = 0;
	// body
	(*msg)->body = (char*)malloc(sizeof(char));
	(*msg)->body[0] = '\0';
	(*msg)->blen = 0;
}

// initialize client socket
// returns data of socket
struct cs_data_t* bind_client_data(int fd, struct sockaddr addr, int state)
{
	struct cs_data_t* data = malloc(sizeof(*data));
	data->fd = fd;
	data->addr = addr;
	data->flw = true;
	data->hmode = true;
	data->state = state;
	init_message_struct(&(data->msg));
	data->buf = malloc(sizeof(*(data->buf)) * BUFFER_SIZE);
	data->offset = 0;
	//printf("Worker(%d): bind data to socket (fd = %d)\n", getpid(), fd);
	return data;
}

// free data of client
void free_client_data(struct cs_data_t** data)
{
	if (close((*data)->fd) != 0)	// close socket
		parse_error(ERR_CLOSE);
	free_message_struct(&(*data)->msg);
	free(*data);
	*data = NULL;
}

// close clients socket by state
// state = SOCKET_NOSTATE - close all sockets in list
int close_client_sockets_by_state(struct cs_node_t** head, int state)
{
	struct cs_node_t* tmp = *head;
	struct cs_node_t* prev = NULL;
	int count = 0;
	while (tmp != NULL) {
		// socket already deleted, empty struct
		if (tmp->cs == NULL) {
			if (prev == NULL) {	// move head
				*head = tmp->next;
				free(tmp);
				tmp = *head;
			} else {
				prev->next = tmp->next;
				free(tmp);
				tmp = prev->next;
			}
			continue;
		}
		// send receive about terminate
		if (tmp->cs->state != SOCKET_STATE_CLOSED && state == SOCKET_NOSTATE)
			send_data(tmp->cs->fd, REPLY_TERMINATE, strlen(REPLY_TERMINATE), 0);
		// delete sockets
		if (tmp->cs->state == state || state == SOCKET_NOSTATE) {
			if (prev == NULL) {	// move head
				*head = tmp->next;
				free_client_data(&tmp->cs);
				free(tmp);
				tmp = *head;
			} else {
				prev->next = tmp->next;
				free_client_data(&tmp->cs);
				free(tmp);
				tmp = prev->next;
			}
			count--;
			continue;
		}
		count++;
		tmp = tmp->next;
	}
	return count;
}

// accept connections
// fd - listen file description
struct cs_data_t* accept_client_socket(int fd)
{
	// accept connection
	struct sockaddr new_addr;
	socklen_t new_len = sizeof(new_addr);
	int new_fd = accept(fd, &new_addr, &new_len);
	if (new_fd < 0) {
		if (errno != EWOULDBLOCK)
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
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		parse_error(ERR_FCNTL);
		return NULL;
	}
	return bind_client_data(new_fd, new_addr, SOCKET_STATE_START);
}