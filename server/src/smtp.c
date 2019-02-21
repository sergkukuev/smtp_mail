#include "handlers.h"
#include "def_smpt.h"
#include "smtp.h"

#include <unistd.h>

// key word parser
int parse_key_word(char* key)
{
    key[4] = '\0';  // crutch for compare
    if (strcmp(key, STR_HELO) == 0)
        return KEY_HELO;
    if (strcmp(key, STR_EHLO) == 0)
        return KEY_EHLO;
    if (strcmp(key, STR_MAIL) == 0)
        return KEY_MAIL;
    if (strcmp(key, STR_RCPT) == 0)
        return KEY_RCPT;
    if (strcmp(key, STR_DATA) == 0)
        return KEY_DATA;
    if (strcmp(key, STR_NOOP) == 0)
        return KEY_NOOP;
    if (strcmp(key, STR_RSET) == 0)
        return KEY_RSET;
    if (strcmp(key, STR_QUIT) == 0)
        return KEY_QUIT;
    if (strcmp(key, STR_VRFY) == 0)
        return KEY_VRFY;
    return KEY_FAILED;
}

int key_switcher(struct cs_data_t* cs, char* msg, bool* quit, mqd_t lg) 
{
    int err = 0;
    bool undef = false;
    switch(parse_key_word(cs->buf)) {
    case KEY_HELO:
        err = HELO_handle(cs, msg + 5, false);
        break;
    case KEY_EHLO:
        err = EHLO_handle(cs, msg + 5);
        break;
    case KEY_MAIL:
        err = MAIL_handle(cs, msg + 5);
        break;
    case KEY_RCPT:
        err = RCPT_handle(cs, msg + 5);
        break;
    case KEY_DATA:
        err = DATA_handle(cs, msg + 5);
        break;
    case KEY_NOOP:
        err = NOOP_handle(cs);
        break;
    case KEY_RSET:
        err = RSET_handle(cs, msg + 5);
        break;
    case KEY_VRFY:
        err = VRFY_handle(cs, msg + 5);
        break;
    case KEY_QUIT:  // exit session
        err = QUIT_handle(cs, msg + 5);
        *quit = true;
        break;
    // undefined 
    default: {
        char bf[BUFFER_SIZE];
        sprintf(bf, "command undefined(socket:%d)\n", cs->fd);
        mq_log(lg, bf);
        undef = true;
        UNDEFINED_handle(cs);
    }
    }
    if (!undef) {
        char bf[BUFFER_SIZE];
        sprintf(bf, "good parse command(socket:%d)\n", cs->fd);
        mq_log(lg, bf);
    }
    return err;
}

// parse key word and send result
// send message
void reply_handle(struct cs_data_t* cs, mqd_t lg) 
{
    bool bq = false;    // quit flag
    while(strstr(cs->buf, "\r\n")) {
        char* eol = strstr(cs->buf, "\r\n");
        eol[0] = '\0';
        printf("Server(%d): client(%d) send msg < %s >\n", getpid(), cs->fd, cs->buf);
        char* msg = (char*) malloc(BUFFER_SIZE);
        strcpy(msg, cs->buf);
        /* int res = */ (cs->state == SOCKET_STATE_DATA) ? TEXT_handle(cs, msg) : key_switcher(cs, msg, &bq, lg);
        free(msg);
        if (bq) break;  // quit session
        // if (res == DATA_FAILED) ALLOWED_handle(cs);
        memmove(cs->buf, eol + 2, BUFFER_SIZE - (eol + 2 - cs->buf));
    }
    // set readfds flag
    cs->fl_write = false;
}

// receive message
void accept_handle(struct cs_data_t* cs, int bf_left, mqd_t lg)
{
    switch (recv_data(cs->fd, cs->buf + cs->offset_buf, bf_left, 0)) {
    case DATA_BLOCK:
        break;
    case DATA_FAILED:
    case DATA_EMPTY:
        cs->state = SOCKET_STATE_CLOSED;
        break;
    default:
        if (strstr(cs->buf, "\r\n"))    cs->fl_write = true;
        break;
    }
}

// main smtp handler (can parse all command)
void main_handle(struct cs_data_t* cs, mqd_t lg)
{
    // log
    char bf[BUFFER_SIZE];
    sprintf(bf, "handle started for socket(%d)\n", cs->fd);
    mq_log(lg, bf);
    // send greeting
    if (cs->state == SOCKET_STATE_START) {
        char bf[BUFFER_SIZE];
        sprintf(bf, "%s", RSMTP_HELLO);
        switch(send_data(cs->fd, bf, strlen(bf), 0)) {
        case DATA_BLOCK:
            break;
        case DATA_FAILED:
            cs->state = SOCKET_STATE_CLOSED;
            break;
        default:
            cs->state = SOCKET_STATE_INIT;
            cs->fl_write = false;
            break;
        }
        return;
    }
    // buffer fill
    int bf_left = BUFFER_SIZE - cs->offset_buf - 1;
    if (bf_left == 0) {
        sprintf(bf, RSMTP_500_FILLED);
        switch(send_data(cs->fd, bf, strlen(bf), 0)) {
        case DATA_BLOCK:
            break;
        case DATA_FAILED:
            cs->state = SOCKET_STATE_CLOSED;
            break;
        default:
            cs->offset_buf = 0;
            cs->fl_write = false;
            break;
        }
    }
    // send or receive
    cs->fl_write ? reply_handle(cs, lg) : accept_handle(cs, bf_left, lg);
}