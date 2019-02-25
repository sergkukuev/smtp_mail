#include "handlers.h"
#include "smtp_def.h"
#include "sockets.h"

// base handle
int HELO_handle(struct cs_data_t* cs, char* msg)
{
    int result = DATA_FAILED;
    char bf[BUFFER_SIZE];
    sprintf(bf, "250- Hello %s\r\n", msg);
    //socklen_t addrlen = sizeof(cs->addr);
    //getpeername(cs->fd, (struct sockaddr*) &cs->addr, &addrlen); 
    result = send_data(cs->fd, bf, strlen(bf), 0);
    if (result >= 0 && cs->state == SOCKET_STATE_INIT)    // change socket state
        cs->state = SOCKET_STATE_WAIT;
    if (result >= 0)    // set helo mode
        cs->hmode = true;
    return result;
}

int EHLO_handle(struct cs_data_t* cs, char* msg) 
{
    int result = DATA_FAILED;
    char bf[BUFFER_SIZE];
    sprintf(bf, "250- Hello %s\r\n%s", msg, REPLY_EHLO);
    result = send_data(cs->fd, bf, strlen(bf), 0);
    if (result >= 0 && cs->state == SOCKET_STATE_INIT)
        cs->state = SOCKET_STATE_WAIT;
    if (result >= 0)
        cs->hmode = false;  // set ehlo mode
    return result;
}

int MAIL_handle(struct cs_data_t* cs, char* msg)
{
    int result = DATA_FAILED;
    switch (cs->state) {
    case SOCKET_STATE_WAIT: {
        // TODO: check fall connections
        char* from = cs->msg->from = parse_mail(msg);
        char bf[BUFFER_SIZE] = REPLY_OK;
        if (strlen(from) == 0)
            sprintf(bf, "%s", REPLY_UN_MAIL);
        result = send_data(cs->fd, bf, strlen(bf), 0);
        if (result >= 0 && strcmp(bf, REPLY_OK) == 0)    // change socket state
            cs->state = SOCKET_STATE_MAIL;
        break;
    }
    default:
        result = send_data(cs->fd, REPLY_SEQ, strlen(REPLY_SEQ), 0);
        break;
    }
    return result;
}

int RCPT_handle(struct cs_data_t* cs, char* msg)
{
    int result = DATA_FAILED;
    switch (cs->state) {
    case SOCKET_STATE_MAIL:
    case SOCKET_STATE_RCPT: {
        // check maximal recepients
        char bf[BUFFER_SIZE] = REPLY_MUCH_REC;
        if (cs->msg->rnum != 10) {
            // TODO: check fall connections
            char* to = cs->msg->to[cs->msg->rnum] = parse_mail(msg);
            if (strlen(to) != 0) {
                cs->msg->rnum++;
                sprintf(bf, "%s", REPLY_OK);
            } else {
                sprintf(bf, "%s", REPLY_UN_MAIL);
            }
        }
        result = send_data(cs->fd, bf, strlen(bf), 0);
        if (result >= 0 && strcmp(bf, REPLY_OK) == 0)    // change socket state
            cs->state = SOCKET_STATE_RCPT;
        break;
    }
    default:
        result = send_data(cs->fd, REPLY_SEQ, strlen(REPLY_SEQ), 0);
        break;
    }
    return result;
}

int DATA_handle(struct cs_data_t* cs, char* msg)
{
    int result = DATA_FAILED;
    switch (cs->state) {
    case SOCKET_STATE_RCPT: {
        //if (strcmp(msg, "") == 0 || msg == NULL) {
        result = send_data(cs->fd, REPLY_DATA, strlen(REPLY_DATA), 0);
        if (result >= 0)
            cs->state = SOCKET_STATE_DATA;
        //} else {
        //    result = send_data(cs->fd, RSMTP_501, strlen(RSMTP_501), 0);
        //}
        break;
    }
    default:
        result = send_data(cs->fd, REPLY_SEQ, strlen(REPLY_SEQ), 0);
        break;
    }
    return result;
}

int NOOP_handle(struct cs_data_t* cs)
{
    return cs->hmode ? send_data(cs->fd, REPLY_ADMIN, strlen(REPLY_ADMIN), 0) : send_data(cs->fd, REPLY_OK, strlen(REPLY_OK), 0);
}

int RSET_handle(struct cs_data_t* cs, char* msg)
{
    int result = DATA_FAILED;
    if (cs->hmode) {
        result = send_data(cs->fd, REPLY_ADMIN, strlen(REPLY_ADMIN), 0);
    } else {
        result = send_data(cs->fd, REPLY_OK, strlen(REPLY_OK), 0);
        if (result >= 0) {
            cs->state = SOCKET_STATE_WAIT;
            clear_message(cs->msg);
        }
    }
    return result;
}

int VRFY_handle(struct cs_data_t* cs, char* msg)
{
    return send_data(cs->fd, REPLY_ADMIN, strlen(REPLY_ADMIN), 0);
}

int QUIT_handle(struct cs_data_t* cs, char* msg)
{
    int result = DATA_FAILED;
    //if (strcmp(msg, "") == 0 || msg == NULL) {
    result = send_data(cs->fd, REPLY_QUIT, strlen(REPLY_QUIT), 0);
    if (result >= 0) {   // change socket state
        cs->state = SOCKET_STATE_CLOSED;
    }
    //} else {
    //    result = send_data(cs->fd, REPLY_ARGS, strlen(REPLY_ARGS), 0);
    //}
    return result;
}

// writing message
int TEXT_handle(struct cs_data_t* cs, char* msg)
{
    int result = 0;
    if (strcmp(cs->buf, ".") != 0) {
        if (strlen(cs->msg->body) + strlen(cs->buf) >= cs->msg->blen) {
            // reallocate
            int sz = strlen(cs->msg->body) + BUFFER_SIZE * 2;
            cs->msg->body = (char*) realloc(cs->msg->body, sz);
            cs->msg->blen = sz; 
        }
        strcat(cs->msg->body, cs->buf);
        // sprintf(cs->msg->body + cs->msg->blen, "\n");
        cs->msg->body[cs->msg->blen] = '\n';
        *(cs->msg->body + cs->msg->blen + 1) = '\0';
    } else {
        // end message
        // printf("\n\n%s\n\n", cs->msg->body);
        // TODO: check fall connections
        save_message(cs->msg);
        result = RSET_handle(cs, NULL); // call rset function after save message
    }
    return result;
}

// undefined
int UNDEFINED_handle(struct cs_data_t* cs)
{
    return send_data(cs->fd, REPLY_UNREC, strlen(REPLY_UNREC), 0);
}