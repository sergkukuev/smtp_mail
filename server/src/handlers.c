#include "handlers.h"

// base handle
int HELO_handle(struct cs_data_t* cs)
{
    char buf[] = "HELO";
    if (cs->fd > 0)
        if (send(cs->fd, buf, strlen(buf), 0) < 0)
            if (errno == EWOULDBLOCK)
                return 1;
    return 0;
}

int EHLO_handle(struct cs_data_t* cs) 
{
    char buf[] = "HELO";
    if (cs->fd > 0)
        if (send(cs->fd, buf, strlen(buf), 0) < 0)
            if (errno == EWOULDBLOCK)
                return 1;
    return 0;
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
    char buf[] = "HELO";
    if (cs->fd > 0)
        if (send(cs->fd, buf, strlen(buf), 0) < 0)
            if (errno == EWOULDBLOCK)
                return 1;
    return 0;
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

// connect with maildir folder
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
    char buf[] = "HELO";
    if (cs->fd > 0)
        if (send(cs->fd, buf, strlen(buf), 0) < 0)
            if (errno == EWOULDBLOCK)
                return 1;
    return 0;
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