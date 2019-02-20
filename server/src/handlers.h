#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include "common.h"

// base handlers
int HELO_handle(struct cs_data_t* cs);
int EHLO_handle(struct cs_data_t* cs);
int MAIL_handle(struct cs_data_t* cs);
int RCPT_handle(struct cs_data_t* cs);
int DATA_handle(struct cs_data_t* cs);
int NOOP_handle(struct cs_data_t* cs);
int RSET_handle(struct cs_data_t* cs);
int QUIT_handle(struct cs_data_t* cs);

// connect with maildir folder
int TEXT_handle(struct cs_data_t* cs);

// undefined command
int UNDEFINED_handle(struct cs_data_t* cs);

// send client allowed command list
int ALLOWED_handle(struct cs_data_t* cs);

#endif