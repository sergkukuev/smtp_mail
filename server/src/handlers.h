#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include "common.h"

// base handlers
int HELO_handle(struct cs_data_t* cs, char* msg);
int EHLO_handle(struct cs_data_t* cs, char* msg);
int MAIL_handle(struct cs_data_t* cs, char* msg);
int RCPT_handle(struct cs_data_t* cs, char* msg);
int DATA_handle(struct cs_data_t* cs, char* msg);
int VRFY_handle(struct cs_data_t* cs, char* msg);
int NOOP_handle(struct cs_data_t* cs);
int RSET_handle(struct cs_data_t* cs, char* msg);
int QUIT_handle(struct cs_data_t* cs, char* msg);

// connect with maildir folder
int TEXT_handle(struct cs_data_t* cs, char* msg);

// undefined command
int UNDEFINED_handle(struct cs_data_t* cs);

#endif