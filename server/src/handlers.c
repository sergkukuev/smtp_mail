#include "handlers.h"
#include "def_smpt.h"

// sending data
int send_data(int fd, char* bf, size_t bfsz, int flags)
{
    if (fd <= 0)    return DATA_FAILED;
    int nbytes = send(fd, bf, bfsz, flags);
    return nbytes < 0 ? ((errno == EWOULDBLOCK) ? DATA_BLOCK : DATA_FAILED) : nbytes;
}

// recv data
int recv_data(int fd, char* bf, size_t bfsz, int flags)
{
    if (fd <= 0)    return DATA_FAILED;
    int nbytes = recv(fd, bf, bfsz, flags);
    return nbytes < 0 ? ((errno == EWOULDBLOCK) ? DATA_BLOCK : DATA_FAILED) : nbytes;
}

// base handle
int HELO_handle(struct cs_data_t* cs, char* msg)
{
    int result = DATA_FAILED;
    switch (cs->state) {
    case SOCKET_STATE_INIT: {
        char bf[BUFFER_SIZE] = RSMTP_250;
        struct sockaddr_in addr;
        socklen_t addrlen;
        get_address(&addr, &addrlen);
        getpeername(cs->fd, (struct sockaddr*) &addr, &addrlen); 
        result = send_data(cs->fd, bf, sizeof(bf), 0);
        if (result >= 0)    // change socket state
            cs->state = SOCKET_STATE_WAIT;
    }
    default:
        break;
    }
    return result;
}

int EHLO_handle(struct cs_data_t* cs, char* msg) 
{
    return HELO_handle(cs, msg);
}

int MAIL_handle(struct cs_data_t* cs, char* msg)
{
    int result = DATA_FAILED;
    switch (cs->state) {
    case SOCKET_STATE_WAIT: {
        char* from = cs->message->from = msg;
        char bf[BUFFER_SIZE] = RSMTP_450;
        if ((from != NULL) & (strcmp(from, "") != 0))
            sprintf(bf, "%s", RSMTP_250);
        result = send_data(cs->fd, bf, sizeof(bf), 0);
        if (result >= 0 && strcmp(bf, RSMTP_250) == 0)    // change socket state
            cs->state = SOCKET_STATE_MAIL;
    }
    default:
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
        char bf[BUFFER_SIZE] = "too much recepients";//RSMTP_451;
        if (cs->message->rnum != 10) {
            char* to = cs->message->to[cs->message->rnum] = msg;
            if (to != NULL) {
                cs->message->rnum++;
                sprintf(bf, "%s", RSMTP_250);
            } else {
                sprintf(bf, "%s", RSMTP_450);
            }
        }
        result = send_data(cs->fd, bf, sizeof(bf), 0);
        if (result >= 0 && strcmp(bf, RSMTP_250) == 0)    // change socket state
            cs->state = SOCKET_STATE_RCPT;
    }
    default:
        break;
    }
    return result;
}

int DATA_handle(struct cs_data_t* cs)
{
    int result = DATA_FAILED;
    switch (cs->state) {
    case SOCKET_STATE_RCPT: {
        char bf[BUFFER_SIZE] = RSMTP_354;
        result = send_data(cs->fd, bf, sizeof(bf), 0);
        if (result >= 0) { 
            cs->state = SOCKET_STATE_DATA;
            cs->message->body = (char*) malloc(1);
            cs->message->body[0] = '\0';
            cs->message->blen = 0;
        }
    }
    default:
        break;
    }
    return result;
}

int NOOP_handle(struct cs_data_t* cs)
{
    char bf[BUFFER_SIZE] = RSMTP_250;
    return send_data(cs->fd, bf, sizeof(bf), 0);
}

int RSET_handle(struct cs_data_t* cs)
{
    int result = DATA_FAILED;
    switch (cs->state) {
    case SOCKET_STATE_TEXT:
    case SOCKET_STATE_MAIL:
    case SOCKET_STATE_RCPT:
    case SOCKET_STATE_WAIT: {
        // full clean
        free(cs->message->from);
        cs->message->from = NULL;
        for (int i = 0; i <= cs->message->rnum; i++) {
            free(cs->message->to[i]);
            cs->message->to[i] = NULL;
        }
        free(cs->message->body);
        char bf[BUFFER_SIZE] = RSMTP_250;
        result = send_data(cs->fd, bf, sizeof(bf), 0);
        if (result >= 0)    // change socket state
            cs->state = SOCKET_STATE_WAIT;
    }
    default:
        break;
    }
    return result;
}

int QUIT_handle(struct cs_data_t* cs)
{
    char bf[BUFFER_SIZE] = RSMTP_221;
    int nbytes = send_data(cs->fd, bf, sizeof(bf), 0);
    if (nbytes >= 0)    // change socket state
        cs->state = SOCKET_STATE_CLOSED;
    return nbytes;
}

// writing message
int TEXT_handle(struct cs_data_t* cs, char* msg)
{
    int result = 0;
    if (strcmp(cs->buf, ".") != 0) {
        if (strlen(cs->message->body) + strlen(cs->buf) >= cs->message->blen) {
            // reallocate
            int sz = strlen(cs->message->body) + BUFFER_SIZE * 2;
            cs->message->body = (char*)realloc(cs->message->body, sz);
            cs->message->blen = sz; 
        }
        strcat(cs->message->body, cs->buf);
        cs->message->body[cs->message->blen] = "\n";
        cs->message->body[cs->message->blen + 1] = "\0";
    } else {
        // end message
        save_message(cs->message);
        cs->state = SOCKET_STATE_TEXT;
        result = RSET_handle(cs);
    }
    return result;
}

// undefined
int UNDEFINED_handle(struct cs_data_t* cs)
{
    char bf[BUFFER_SIZE] = RSMTP_500;
    return send_data(cs->fd, bf, sizeof(bf), 0);
}

// send client allowed command list
int ALLOWED_handle(struct cs_data_t* cs)
{
    char stub[] = "Nope";
    return send_data(cs->fd, stub, sizeof(stub), 0);
}