#ifndef __COMMON_H__
#define __COMMON_H__

#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
// DEFINES

// server data
#define SERVER_ADDR "0.0.0.0"
#define SERVER_PORT "2525"
#define SERVER_DOMAIN "myserver.ru"
#define BUFFER_SIZE 1024
#define BACKLOG_SIZE 3
#define SELECT_TIMEOUT 60

// socket state
#define SOCKET_NOSTATE -1
#define SOCKET_STATE_INIT 0
#define SOCKET_STATE_WAIT 1
#define SOCKET_STATE_MAIL 2
#define SOCKET_STATE_RCPT 3
#define SOCKET_STATE_DATA 4
#define SOCKET_STATE_TEXT 5
#define SOCKET_STATE_CLOSED 5
#define SOCKET_STATE_START 6

// maildir 
#define PARSE_FAILED -1
#define MAILDIR "../maildir/"
#define MAILSTART '<'
#define MAILEND '>'

//struct of message
struct msg_t {
    char** to;
    char* from;
    char* body;
    int blen;   // body length
    int rnum;   // recepients numeric
};

// data of client socket
struct cs_data_t {
    int fd;
    int state;
    char* buf;
    bool flw;
    int offset;
    struct msg_t* msg;
};

// node of client socket
struct cs_node_t {
    struct cs_data_t* cs;
    struct cs_node_t* next;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// FUNCTIONS

void get_address(struct sockaddr_in* addr, socklen_t* addrlen);
char* parse_mail(char* bf);
int save_message(struct msg_t* msg);
int mq_log(int lg, char* msg);

#endif // !__COMMON_H__
