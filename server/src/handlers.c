#include "handlers.h"
#include "smtp_def.h"

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
int HELO_handle(struct cs_data_t* cs, char* msg, bool ehlo)
{
    int result = DATA_FAILED;
    char bf[BUFFER_SIZE];
    (!ehlo) ? sprintf(bf, "%sHello %s \r\n", RSMTP_250_TEXT, msg) :
        sprintf(bf, "%sHello %s\r\n%sPIPELINING\r\n%sHELP\r\n", RSMTP_250_TEXT, msg, RSMTP_250_TEXT, RSMTP_250_TEXT);
    socklen_t addrlen = sizeof(cs->addr);
    getpeername(cs->fd, (struct sockaddr*) &cs->addr, &addrlen); 
    result = send_data(cs->fd, bf, strlen(bf), 0);
    if (result >= 0 && cs->state == SOCKET_STATE_INIT)    // change socket state
        cs->state = SOCKET_STATE_WAIT;
    return result;
}

int EHLO_handle(struct cs_data_t* cs, char* msg) 
{
    return HELO_handle(cs, msg, true);
}

int MAIL_handle(struct cs_data_t* cs, char* msg)
{
    int result = DATA_FAILED;
    switch (cs->state) {
    case SOCKET_STATE_WAIT: {
        if (cs->msg == NULL)    init_message(&cs->msg); // message initialization
        char* from = cs->msg->from = parse_mail(msg);
        char bf[BUFFER_SIZE] = RSMTP_250;
        if (from == NULL)   sprintf(bf, "%s", RSMTP_450);
        result = send_data(cs->fd, bf, strlen(bf), 0);
        if (result >= 0 && strcmp(bf, RSMTP_250) == 0)    // change socket state
            cs->state = SOCKET_STATE_MAIL;
        break;
    }
    default:
        result = send_data(cs->fd, RSMTP_503, strlen(RSMTP_503), 0);
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
        char bf[BUFFER_SIZE] = RSMTP_451;
        if (cs->msg->rnum != 10) {
            char* to = cs->msg->to[cs->msg->rnum] = parse_mail(msg);
            if (to != NULL) {
                cs->msg->rnum++;
                sprintf(bf, "%s", RSMTP_250_RCPT);
            } else {
                sprintf(bf, "%s", RSMTP_450);
            }
        }
        result = send_data(cs->fd, bf, strlen(bf), 0);
        if (result >= 0 && strcmp(bf, RSMTP_250_RCPT) == 0)    // change socket state
            cs->state = SOCKET_STATE_RCPT;
        break;
    }
    default:
        result = send_data(cs->fd, RSMTP_503, strlen(RSMTP_503), 0);
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
            char bf[BUFFER_SIZE] = RSMTP_354;
            result = send_data(cs->fd, bf, strlen(bf), 0);
            if (result >= 0) { 
                cs->state = SOCKET_STATE_DATA;
                cs->msg->body = (char*) malloc(1);
                cs->msg->body[0] = '\0';
                cs->msg->blen = 0;
            }
        //} else {
        //    result = send_data(cs->fd, RSMTP_501, strlen(RSMTP_501), 0);
        //}
        break;
    }
    default:
        result = send_data(cs->fd, RSMTP_503, strlen(RSMTP_503), 0);
        break;
    }
    return result;
}

int NOOP_handle(struct cs_data_t* cs)
{
    char bf[BUFFER_SIZE] = RSMTP_250;
    return send_data(cs->fd, bf, strlen(bf), 0);
}

int RSET_handle(struct cs_data_t* cs, char* msg)
{
    int result = DATA_FAILED;
    //if (msg == NULL || strcmp(msg, "") == 0) {
        char bf[BUFFER_SIZE] = RSMTP_250_RESET;
        result = send_data(cs->fd, bf, strlen(bf), 0);
        if (result >= 0) {
            cs->state = SOCKET_STATE_WAIT;
            free_message(&cs->msg);     // free message structure
        }       
    //} else {
    //    result = send_data(cs->fd, RSMTP_501, strlen(RSMTP_501), 0);
    //}
    return result;
}

int VRFY_handle(struct cs_data_t* cs, char* msg)
{
    return send_data(cs->fd, RSMTP_252_VRFY, strlen(RSMTP_252_VRFY), 0);
}

int QUIT_handle(struct cs_data_t* cs, char* msg)
{
    int result = DATA_FAILED;
    //if (strcmp(msg, "") == 0 || msg == NULL) {
        char bf[BUFFER_SIZE] = RSMTP_221;
        result = send_data(cs->fd, bf, strlen(bf), 0);
        if (result >= 0)    // change socket state
            cs->state = SOCKET_STATE_CLOSED;
    //} else {
    //    result = send_data(cs->fd, RSMTP_501, strlen(RSMTP_501), 0);
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
            cs->msg->body = (char*)realloc(cs->msg->body, sz);
            cs->msg->blen = sz; 
        }
        strcat(cs->msg->body, cs->buf);
        // sprintf(cs->msg->body + cs->msg->blen, "\n");
        cs->msg->body[cs->msg->blen] = '\n';
        *(cs->msg->body + cs->msg->blen + 1) = '\0';
    } else {
        // end message
        save_message(cs->msg);
        cs->state = SOCKET_STATE_TEXT;
        result = RSET_handle(cs, NULL);
    }
    return result;
}

// undefined
int UNDEFINED_handle(struct cs_data_t* cs)
{
    char bf[BUFFER_SIZE] = RSMTP_500;
    return send_data(cs->fd, bf, strlen(bf), 0);
}