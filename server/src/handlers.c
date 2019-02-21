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

int MAIL_handle(struct cs_data_t* cs)
{
    char buf[] = "HELO";
    if (cs->fd > 0)
        if (send(cs->fd, buf, strlen(buf), 0) < 0)
            if (errno == EWOULDBLOCK)
                return 1;
    return 0;
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
    char buf[] = "HELO";
    if (cs->fd > 0)
        if (send(cs->fd, buf, strlen(buf), 0) < 0)
            if (errno == EWOULDBLOCK)
                return 1;
    return 0;
}

int QUIT_handle(struct cs_data_t* cs)
{
    char buf[] = "HELO";
    if (cs->fd > 0)
        if (send(cs->fd, buf, strlen(buf), 0) < 0)
            if (errno == EWOULDBLOCK)
                return 1;
    return 0;
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