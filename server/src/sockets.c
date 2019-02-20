#include "sockets.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

// errors 
#define ERR_SOCKET -1
#define ERR_SOCKOPT -2
#define ERR_BIND -3
#define ERR_LISTEN -4
#define ERR_FCNTL -5

// other defines
#define BACKLOG_SIZE 3

// creating server socket
int create_serv_socket(struct addrinfo* inst)
{
	void* addr = &((struct sockaddr_in*)inst->ai_addr)->sin_addr;
    char ip_str[INET_ADDRSTRLEN];

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
	switch (err)
	{
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
		default:
			perror("undefined error");
			break;
	}
}

// init server sockets by getaddrinfo()
struct ss_node_t* init_serv_sockets(void)
{
	struct ss_node_t* head = NULL;
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
    
    for (struct addrinfo* i = hai; i != NULL; i = i->ai_next) {
		int s_fd = create_serv_socket(i);
		if (s_fd >= 0) {
			// successfully creating	
			printf("socket created (%d)\n", s_fd);
			struct ss_node_t* node = malloc(sizeof &node);
			node->fd = s_fd;
			node->next = head;
			head = node;
		} else {
			serv_sock_error(s_fd);
		}
	}
    freeaddrinfo(hai);
    return head;
}