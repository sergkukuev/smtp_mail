#ifndef __SMTP_H__
#define __SMTP_H__

#include "common.h"

// main smtp handler (can parse all command)
void main_handle(struct cs_data_t* cs);

#endif // !__SMTP_H__