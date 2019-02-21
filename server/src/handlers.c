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
        char bf[BUFFER_SIZE] = (from != NULL && strcmp(from, "") != 0) ? RSMTP_250 : RSMTP_450;
        result = send_data(cs->fd, bf, sizeof(bf), 0);
        if (result >= 0 && strcmp(bf, RSMTP_250) == 0)    // change socket state
            cs->state = SOCKET_STATE_MAIL;
    }
    default:
        break;
    }
    return result;
}

int RCPT_handle(struct cs_data_t* cs)
{
    char buf[] = "HELO";
    if (cs->fd > 0)
        if (send(cs->fd, buf, strlen(buf), 0) < 0)
            if (errno == EWOULDBLOCK)
                return 1;
    return 0;
}

int DATA_handle(struct cs_data_t* cs)
{
    char buf[] = "HELO";
    if (cs->fd > 0)
        if (send(cs->fd, buf, strlen(buf), 0) < 0)
            if (errno == EWOULDBLOCK)
                return 1;
    return 0;
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
int TEXT_handle(struct cs_data_t* cs)
{
    char buf[] = "HELO";
    if (cs->fd > 0)
        if (send(cs->fd, buf, strlen(buf), 0) < 0)
            if (errno == EWOULDBLOCK)
                return 1;
    return 0;
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
    char buf[] = "HELO";
    if (cs->fd > 0)
        if (send(cs->fd, buf, strlen(buf), 0) < 0)
            if (errno == EWOULDBLOCK)
                return 1;
    return 0;
}