#include "utils.h"
#include "../../src/handlers.h"
#include "../../src/sockets.h"

MYTEST(full_correct_session)
{
    char* msg = "EHLO localhost";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_INIT);

    int err = EHLO_handle(data, msg + 5);
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_WAIT);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);

    char* msg1 = "MAIL <test@test.ru>";
    err = MAIL_handle(data, msg1 + 5);
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_MAIL);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);

    char* msg2 = "RCPT <test@test.ru>";
    err = RCPT_handle(data, msg2 + 5);
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_RCPT);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);

    char* msg3 = "DATA";
    err = DATA_handle(data, msg3+5);
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_DATA);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);

    char* msg4 = "awdawdnawd";
    err = TEXT_handle(data, msg4);
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_DATA);
    //CU_ASSERT_EQUAL(data->msg->body, msg4)
    
    err = TEXT_handle(data, ".");
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_WAIT);
    //CU_ASSERT_EQUAL(err, DATA_NOT_SEND);

    err = QUIT_handle(data, "");
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_CLOSED);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_helo_correct)
{
    char* msg = "HELO server.ru";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_INIT);

    int err = HELO_handle(data, msg + 5);
    
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_WAIT);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_helo_incorrect)
{
    char* msg = "HELO server.ru";
    struct sockaddr addr;
    struct cs_data_t* data = NULL;

    int err = HELO_handle(data, msg + 5);
    
    CU_ASSERT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(err, DATA_S_EMPTY);
}

MYTEST(handle_ehlo_correct)
{
    char* msg = "EHLO server.ru";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_INIT);

    int err = EHLO_handle(data, msg + 5);
    
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_WAIT);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_ehlo_incorrect)
{
    char* msg = "HELO server.ru";
    struct sockaddr addr;
    struct cs_data_t* data = NULL;

    int err = HELO_handle(data, msg + 5);
    
    CU_ASSERT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(err, DATA_S_EMPTY);
}

MYTEST(handle_mail_correct)
{
    char* msg = "MAIL <test@test.ru>";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_WAIT);

    int err = MAIL_handle(data, msg + 5);
    
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_MAIL);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_mail_incorrect_state)
{
    char* msg = "MAIL <test@test.ru>";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_INIT);

    int err = MAIL_handle(data, msg + 5);
    
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_INIT);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_mail_incorrect)
{
    char* msg = "MAIL testawdawd";
    struct sockaddr addr;
    struct cs_data_t* data = NULL;

    int err = MAIL_handle(data, msg + 5);
    
    CU_ASSERT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(err, DATA_S_EMPTY);
}

MYTEST(handle_rcpt_correct)
{
    char* msg = "RCPT <test@test.ru>";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_MAIL);

    int err = RCPT_handle(data, msg + 5);
    
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_RCPT);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_rcpt_incorrect_state)
{
    char* msg = "RCPT <test@test.ru>";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_INIT);

    int err = RCPT_handle(data, msg + 5);
    
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_INIT);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_rcpt_incorrect)
{
    char* msg = "RCPT testawdawd";
    struct sockaddr addr;
    struct cs_data_t* data = NULL;

    int err = RCPT_handle(data, msg + 5);
    
    CU_ASSERT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(err, DATA_S_EMPTY);
}

MYTEST(handle_data_correct)
{
    char* msg = "DATA";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_RCPT);

    int err = DATA_handle(data, msg + 5);
    
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_DATA);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_data_incorrect_state)
{
    char* msg = "DATA";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_INIT);

    int err = DATA_handle(data, msg + 5);
    
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_INIT);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_data_incorrect)
{
    char* msg = "DATA aw";
    struct sockaddr addr;
    struct cs_data_t* data = NULL;

    int err = DATA_handle(data, msg + 5);
    
    CU_ASSERT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(err, DATA_S_EMPTY);
}

MYTEST(handle_rset_correct)
{
    char* msg1 = "EHLO awdawd";
    char* msg2 = "RSET";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_RCPT);
    
    int err = EHLO_handle(data, msg1 + 5);
    err = RSET_handle(data, msg2 + 5);
    
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_WAIT);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_rset_incorrect)
{
    char* msg = "RSET awdad";
    struct sockaddr addr;
    struct cs_data_t* data = NULL;

    int err = RSET_handle(data, msg + 5);
    
    CU_ASSERT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(err, DATA_S_EMPTY);
}

MYTEST(handle_vrfy_correct)
{
    char* msg = "VRFY";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_MAIL);

    int err = DATA_handle(data, msg + 5);
    
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_MAIL);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_vrfy_incorrect)
{
    char* msg = "VRFY awawdawd";
    struct sockaddr addr;
    struct cs_data_t* data = NULL;

    int err = VRFY_handle(data, msg + 5);
    
    CU_ASSERT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(err, DATA_S_EMPTY);
}

MYTEST(handle_quit_correct)
{
    char* msg = "QUIT";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_MAIL);

    int err = QUIT_handle(data, msg + 5);
    
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_CLOSED);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_quit_incorrect)
{
    char* msg = "QUIT";
    struct sockaddr addr;
    struct cs_data_t* data = NULL;

    int err = QUIT_handle(data, msg + 5);
    
    CU_ASSERT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(err, DATA_S_EMPTY);
}

MYTEST(handle_noop_correct)
{
    char* msg = "NOOP";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_MAIL);

    int err = NOOP_handle(data);
    
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_MAIL);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_noop_incorrect)
{
    char* msg = "NOOP";
    struct sockaddr addr;
    struct cs_data_t* data = NULL;

    int err = NOOP_handle(data);
    
    CU_ASSERT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(err, DATA_S_EMPTY);
}

MYTEST(handle_undef_correct)
{
    char* msg = "LOL";
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_MAIL);

    int err = UNDEFINED_handle(data);
    
    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_MAIL);
    CU_ASSERT_EQUAL(err, DATA_NOT_SEND);
    free_client_data(&data);
}

MYTEST(handle_undef_incorrect)
{
    char* msg = "LOL";
    struct sockaddr addr;
    struct cs_data_t* data = NULL;

    int err = UNDEFINED_handle(data);
    
    CU_ASSERT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(err, DATA_S_EMPTY);
}