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
#define SELECT_TIMEOUT 240

// socket state
#define SOCKET_NOSTATE -1
#define SOCKET_STATE_INIT 0
#define SOCKET_STATE_WAIT 1
#define SOCKET_STATE_MAIL 2
#define SOCKET_STATE_RCPT 3
#define SOCKET_STATE_DATA 4
#define SOCKET_STATE_CLOSED 5
#define SOCKET_STATE_START 6

// maildir 
#define MAX_RECIPIENTS 10
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
    struct sockaddr addr;
    int state;
    char* buf;
    bool hmode;  // true - helo, false - ehlo
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

// save message to file in root folder
bool save_to_file(char* fname, char* msg, bool info);

// send message to log message queue
int mq_log(int lg, char* msg);

// function of clear msg_t
void clear_message(struct msg_t* msg);

char* parse_mail(char* bf);
int save_message(struct msg_t* msg);

#endif // !__COMMON_H__
