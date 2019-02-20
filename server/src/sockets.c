#include "sockets.h"
#include "handlers.h"

#include <sys/types.h>
#include <netdb.h>
#include <fcntl.h>

// errors 
#define ERR_SOCKET -1
#define ERR_SOCKOPT -2
#define ERR_BIND -3
#define ERR_LISTEN -4
#define ERR_FCNTL -5
#define ERR_ACCEPT -6

// creating server socket
int create_serv_socket(struct addrinfo* inst)
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
	if (fcntl(s_fd, F_SETFL, flags | O_NONBLOCK))
		return ERR_FCNTL;

	if (bind(s_fd, inst->ai_addr, inst->ai_addrlen) == -1)
		return ERR_BIND;

	if (listen(s_fd, BACKLOG_SIZE) == -1)
		return ERR_LISTEN;

	return s_fd;
}

// parse creating server socket
void serv_sock_error(int err)
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
		default:
			perror("undefined error");
			break;
	}
}

// init server sockets by getaddrinfo()
struct ss_node_t* init_serv_sockets()
{
	struct ss_node_t* head = NULL;
    struct addrinfo* hai;   // host address info pointer
    struct addrinfo sai;    // sockets address info
	char* port = SERVER_PORT;
	// ((struct sockaddr_in*)sai.ai_addr)->sin_addr.s_addr = inet_addr(SERVER_ADDR);

	printf("address: %s\n", SERVER_ADDR);
	printf("port: %s\n", SERVER_PORT);

    memset(&sai, 0, sizeof(sai));
    sai.ai_family = AF_UNSPEC;	// undefined type: ipv4 or ipv6
    sai.ai_socktype = SOCK_STREAM;
    sai.ai_flags = AI_PASSIVE;	// network address no specified

    if (getaddrinfo(NULL, port, &sai, &hai) != 0) {
        perror("getaddrinfo() failed");
        return NULL;
    }
    
    for (struct addrinfo* i = hai; i != NULL; i = i->ai_next) {
		int s_fd = create_serv_socket(i);
		if (s_fd >= 0) {
			// successfully creating	
			struct ss_node_t* node = malloc(sizeof *node);
			node->fd = s_fd;
			node->next = head;
			head = node;
			printf("server socket created (%d)\n", s_fd);
		} else {
			serv_sock_error(s_fd);
		}
	}
    freeaddrinfo(hai);
    return head;
}

// init client sockets
struct cs_data_t create_client_socket(int fd, int state, bool need_msg)
{
	struct cs_data_t cs;
	cs.fd = fd;
	cs.state = state;
	if (need_msg) {
		cs.message = malloc(sizeof *cs.message);
		cs.message->to = malloc(10 * sizeof(char));
		cs.message->from = NULL;
		cs.message->body = NULL;
		cs.message->blen = 0;
		cs.message->rnum = 0;
	}
	return cs;
}

// init one client socket and insert to head
struct cs_node_t* init_client_socket(struct cs_node_t* head, int fd, int state, bool need_msg, int* max_fd)
{
	struct cs_node_t* node = malloc(sizeof *node);
	node->cs = create_client_socket(fd, SOCKET_STATE_WAIT, false);
	node->next = head;
	head = node;
	printf("client socket created (%d)\n", node->cs.fd);
	// calc maximal file descriptor
	if (max_fd != NULL)
		if (node->cs.fd > *(max_fd))
			*(max_fd) = node->cs.fd;
	return head;
}

// init client sockets by server sockets
struct cs_node_t* init_client_sockets(struct ss_node_t* ss_list, int* max_fd)
{
	struct cs_node_t* head = NULL;
	for (struct ss_node_t* i = ss_list; i != NULL; i = i->next)
		head = init_client_socket(head, i->fd, SOCKET_STATE_WAIT, false, max_fd);
	return head;
}

// checkers message queue on exit command
bool check_mq(mqd_t* mq, fd_set* readfds)
{
	if (mq != NULL) {
		// receive message
		if (FD_ISSET(*mq, readfds)) {
			char buf[BUFFER_SIZE];
			memset(buf, 0x00, sizeof(buf));
			int nbytes = mq_receive(*mq, buf, BUFFER_SIZE, NULL);
			if (nbytes > 0) {
				printf("receive message from mq: %s\n", buf);
				if (strcmp(buf, "#") == 0)
					return true;
			}
		}
	}
	return false;
}

// accept connections
void accept_sockets(struct cs_node_t* list, struct cs_node_t* ss_list, fd_set* readfds, int* max_fd)
{
	for (struct cs_node_t* i = list; i != NULL; i = i->next) {
		if (FD_ISSET(i->cs.fd, readfds)) {
			// accept connection
			// TODO: find example, how send address and len
			int fd = -1;//accept(i->cs.fd, address, len);
			if (fd == -1) {
				serv_sock_error(ERR_ACCEPT);
				continue;
			}
			// get flags
			int flags = fcntl(fd, F_GETFL, 0); 
			if (flags == -1) {
				serv_sock_error(ERR_FCNTL);
				continue;
			}
			// set nonblock flag
			if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
				serv_sock_error(ERR_FCNTL);
				continue;
			}
			// initialize
			ss_list = init_client_socket(ss_list, fd, SOCKET_STATE_INIT, true, max_fd);
		}
	}
}

// handle connections
void handle_sockets(struct cs_node_t* list, fd_set* readfds, fd_set* writefds)
{
	for (struct cs_node_t* i = list; i != NULL; i = i->next) {
		if (FD_ISSET(i->cs.fd, readfds)) {
			i->cs.flag = false;
			main_handler(&i->cs);
		}
		if (FD_ISSET(i->cs.fd, writefds)) {
			i->cs.flag = true;
			main_handler(&i->cs);
		}
	}
}

// parser select()
void parse_select(struct process_t* proc)
{
	struct timeval tv;	// timeout for select
	tv.tv_sec = 60;
	tv.tv_usec = 0;

	// call select: can change timeout
	int ndesc = select(proc->max_fd, &(proc->readfds), &(proc->writefds), NULL, &tv);
	switch(ndesc) {
		// error
		case -1:
			perror("select() failed");
			break;
		// no events - close sockets
		case 0:
			printf("timeout select()\n");
			for (struct cs_node_t* i = proc->ss_list; i != NULL; i = i->next)
				if (!FD_ISSET(i->cs.fd, &(proc->readfds)))
					i->cs.state = SOCKET_STATE_CLOSED;
			break;
		// sockets ready - need checks
		default: {
			proc->worked = check_mq(proc->mq, &proc->readfds);
			if (proc->worked) {
				accept_sockets(proc->ls_list, proc->ss_list, &proc->readfds, &proc->max_fd);
				handle_sockets(proc->ss_list, &proc->readfds, &proc->writefds);
			}
		}
	}
}