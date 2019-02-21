#include "handlers.h"
#include "def_smpt.h"
#include "smtp.h"

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
    return KEY_FAILED;
}

// parse key word and send result
// send message
void reply_handle(struct cs_data_t* cs) 
{
    char* eol = strstr(cs->buf, "\r\n");
    while(eol) {
        eol[0] = '\0';
        printf("client: %d, msg: %s\n", cs->fd, cs->buf);
        if (!cs->inpmsg) {
            char* msg = (char*) malloc(BUFFER_SIZE);
            int err = 0;
            strcpy(msg, cs->buf);
            switch(parse_key_word(cs->buf)) {
            case KEY_HELO:
                err = HELO_handle(cs);
                break;
            case KEY_EHLO:
                err = EHLO_handle(cs);
                break;
            case KEY_MAIL:
                err = MAIL_handle(cs);
                break;
            case KEY_RCPT:
                err = RCPT_handle(cs);
                break;
            case KEY_DATA:
                err = DATA_handle(cs);
                break;
            case KEY_NOOP:
                err = NOOP_handle(cs);
                break;
            case KEY_RSET:
                err = RSET_handle(cs);
                break;
            case KEY_QUIT:  // exit session
                err = QUIT_handle(cs);
                return;
            // undefined 
            default:
                UNDEFINED_handle(cs);
            }
            if (err < 0)
                ALLOWED_handle(cs);
            free(msg);
        } else {
            TEXT_handle(cs);
        }
        eol = strstr(cs->buf, "\r\n");
        memmove(cs->buf, eol + 2, BUFFER_SIZE - (eol + 2 - cs->buf));
    }
    // set readfds flag
    cs->flag = false;
}

// receive message
void accept_handle(struct cs_data_t* cs, int bf_left)
{
    int nbytes = recv(cs->fd, cs->buf + cs->offset_buf, bf_left, 0);
    switch (nbytes) {
    case -1:
        if (errno != EWOULDBLOCK)   cs->state = SOCKET_STATE_CLOSED;
        break;
    case 0:
        cs->state = SOCKET_STATE_CLOSED;
        break;
    default:
        if (strstr(cs->buf, "\r\n"))    cs->flag = true;
        break;
    }
}

// main smtp handler (can parse all command)
void main_handle(struct cs_data_t* cs)
{
    // send greeting
    if (cs->state == SOCKET_STATE_START) {
        char bf[BUFFER_SIZE];
        sprintf(bf, "%s %s SMTP CCSMTP\n", RSMTP_220, SERVER_DOMAIN);
        int nbytes = send(cs->fd, bf, strlen(bf), 0);
        switch (nbytes) {
        case -1:
        case 0:
            if (errno != EWOULDBLOCK)   cs->state = SOCKET_STATE_CLOSED;
            break;
        default: 
            cs->state = SOCKET_STATE_INIT;
            cs->flag = false;
            break;
        }
        return;
    }
    // buffer fill
    int bf_left = BUFFER_SIZE - cs->offset_buf - 1;
    if (bf_left == 0) {
        char bf[BUFFER_SIZE];
        sprintf(bf, RSMTP_500);
        int nbytes = send(cs->fd, bf, strlen(bf), 0);
        switch (nbytes) {
        case -1:
        case 0:
            if (errno != EWOULDBLOCK)   cs->state = SOCKET_STATE_CLOSED;
            break;
        default: 
            cs->offset_buf = 0;
            cs->flag = false;
            break;
        }
    }
    // send or receive
    cs->flag ? reply_handle(cs) : accept_handle(cs, bf_left);
}