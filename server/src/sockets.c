#include "sockets.h"
#include "smtp.h"

#include <sys/types.h>
#include <unistd.h>
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

	printf("Server(%d): address - %s\n", getpid(), SERVER_ADDR);
	printf("Server(%d): port - %s\n", getpid(), SERVER_PORT);

    memset(&sai, 0, sizeof(sai));
    sai.ai_family = AF_UNSPEC;	// undefined type: ipv4 or ipv6
    sai.ai_socktype = SOCK_STREAM;
    sai.ai_flags = AI_PASSIVE;	// network address no specified

    if (getaddrinfo(NULL, port, &sai, &hai) != 0) {
        perror("getaddrinfo() failed");
        return NULL;
    }
    
	int num = 0;
    for (struct addrinfo* i = hai; i != NULL; i = i->ai_next) {
		int s_fd = create_serv_socket(i);
		if (s_fd >= 0) {
			// successfully creating	
			struct ss_node_t* node = malloc(sizeof *node);
			node->fd = s_fd;
			node->next = head;
			head = node;
			num++;
			printf("Server(%d): server socket created (fd = %d)\n", getpid(), s_fd);
		} else {
			serv_sock_error(s_fd);
		}
	}
	printf("Server(%d): total server sockets %d\n", getpid(), num);
    freeaddrinfo(hai);
    return head;
}

// init client sockets
struct cs_data_t create_client_socket(int fd, int bfsz, int state, bool need_msg)
{
	struct cs_data_t cs;
	cs.fd = fd;
	cs.state = state;
	cs.fl_write = true;
	cs.buf = (bfsz > 0) ? malloc(bfsz * sizeof(*cs.buf)) : NULL;
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
struct cs_node_t* init_client_socket(struct cs_node_t* head, int fd, int bfsz, int state, bool need_msg, int* max_fd)
{
	struct cs_node_t* node = malloc(sizeof *node);
	node->cs = create_client_socket(fd, bfsz, state, need_msg);
	node->next = head;
	head = node;
	printf("Server(%d): clients socket created (fd = %d)\n", getpid(), node->cs.fd);
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
		head = init_client_socket(head, i->fd, 0, SOCKET_STATE_WAIT, false, max_fd);
	return head;
}

void free_client_socket(struct cs_data_t* sock)
{
	if (sock->message != NULL) {
		for (int i = 0; i < sock->message->rnum; i++) {
			if (sock->message->to[i] != NULL)
				free(sock->message->to[i]);
		}
		if (sock->message->from != NULL)
			free(sock->message->from);
		if (sock->message->body != NULL)
			free(sock->message->body);
		free(sock->message);
	}
	if (sock->buf != NULL)
		free(sock->buf);
	free(sock);
}

struct cs_node_t* delete_sockets_by_state(struct cs_node_t* head, int state)
{
	if (head == NULL)
		return NULL;

	// head delete
	if (head->cs.state == state) {
		struct cs_node_t* new_head = head->next;
		free_client_socket(&head->cs);
		head = new_head;
	}

	// other
	struct cs_node_t* prev = head;
	for(struct cs_node_t* i = head; i != NULL; i = i->next) {
		if (i->cs.state == state) {
			prev->next = i->next;
			free_client_socket(&i->cs);
			continue;
		}
		prev = i;
	}
	return head;
}

int delete_client_sockets(struct cs_node_t** cs_list/*, struct ss_node_t** ss_list*/)
{
	int num = 0;
	// clients
	struct cs_node_t* next = NULL;
	for (struct cs_node_t* i = *cs_list; i != NULL; i = next) {
		num++;
		next = i->next;
		free_client_socket(&i->cs);
	}

	// servers
	/*
	struct ss_node_t* snext = NULL;
	for (struct ss_node_t* i = *ss_list; i != NULL; i = snext) {
		num++;
		snext = i->next;
		free_server_socket(i);
	}
	*/
	*cs_list = NULL;
	//*ss_list = NULL;
	return num;
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
				printf("Server(%d): receive message from mq <%s>\n", getpid(), buf);
				if (strcmp(buf, "#") == 0)
					return false;
			}
		}
	}
	return true;
}

// accept connections
void accept_sockets(struct cs_node_t* list, struct cs_node_t** ss_list, fd_set* readfds, int* max_fd)
{
	for (struct cs_node_t* i = list; i != NULL; i = i->next) {
		if (FD_ISSET(i->cs.fd, readfds)) {
			// accept connection
			struct sockaddr_in addr;
			socklen_t addrlen;
			get_address(&addr, &addrlen);
			int fd = accept(i->cs.fd, (struct sockaddr*) &addr, &addrlen);
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
			*ss_list = init_client_socket(*ss_list, fd, BUFFER_SIZE, SOCKET_STATE_START, true, max_fd);
			// FD_SET(fd, readfds);
		}
	}
}

// handle connections
void handle_sockets(struct cs_node_t* list, fd_set* readfds, fd_set* writefds, mqd_t lg)
{
	for (struct cs_node_t* i = list; i != NULL; i = i->next) {
		if (FD_ISSET(i->cs.fd, readfds)) {
			i->cs.fl_write = false;
			main_handle(&i->cs, lg);
		}
		if (FD_ISSET(i->cs.fd, writefds)) {
			i->cs.fl_write = true;
			main_handle(&i->cs, lg);
		}
	}
}

// parser select()
void parse_select(struct process_t* proc)
{
	struct timeval tv;	// timeout for select
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	/* stub 
	printf("forced determinate\n");
	proc->worked = false;	// forced stop
	return;
	*/
	// call select: can change timeout
	int ndesc = select(proc->max_fd + 1, &(proc->readfds), &(proc->writefds), NULL, &tv);
	switch(ndesc) {
		// error
		case -1:
			perror("select() failed");
			break;
		// no events - close sockets
		case 0:
			printf("Server(%d): Timeout\n", getpid());
			for (struct cs_node_t* i = proc->ss_list; i != NULL; i = i->next)
				if (!FD_ISSET(i->cs.fd, &(proc->readfds)))
					i->cs.state = SOCKET_STATE_CLOSED;
			break;
		// sockets ready - need checks
		default: {
			proc->worked = check_mq(proc->mq, &proc->readfds);
			if (proc->worked) {
				accept_sockets(proc->ls_list, &proc->ss_list, &proc->readfds, &proc->max_fd);
				handle_sockets(proc->ss_list, &proc->readfds, &proc->writefds, proc->lg);
			}
		}
	}
}