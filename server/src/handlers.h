#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include "common.h"

// defines reply and receive
#define DATA_FAILED -2
#define DATA_BLOCK -1
#define DATA_EMPTY 0

// sending data
int send_data(int fd, char* bf, size_t bfsz, int flags);

// receive data
int recv_data(int fd, char* bf, size_t bfsz, int flags);

// base handlers
int HELO_handle(struct cs_data_t* cs, char* msg, bool ehlo);
int EHLO_handle(struct cs_data_t* cs, char* msg);
int MAIL_handle(struct cs_data_t* cs, char* msg);
int RCPT_handle(struct cs_data_t* cs, char* msg);
int DATA_handle(struct cs_data_t* cs, char* msg);
int NOOP_handle(struct cs_data_t* cs);
int RSET_handle(struct cs_data_t* cs, char* msg);
int QUIT_handle(struct cs_data_t* cs);

// connect with maildir folder
int TEXT_handle(struct cs_data_t* cs, char* msg);

// undefined command
int UNDEFINED_handle(struct cs_data_t* cs);

// send client allowed command list
int ALLOWED_handle(struct cs_data_t* cs);

#endif