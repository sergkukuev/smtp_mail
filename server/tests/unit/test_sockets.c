#include "utils.h"
#include "../../src/sockets.h"

MYTEST(socket_listen_correct)
{
    int fd = init_listen_socket();
    CU_ASSERT(fd >= 0);
    int cl = close_listen_socket(fd);
    CU_ASSERT(cl >= 0);
}

MYTEST(socket_bind_client_data_correct)
{
    struct sockaddr addr;
    struct cs_data_t* data = bind_client_data(0, addr, SOCKET_STATE_WAIT);

    CU_ASSERT_NOT_EQUAL(data, NULL);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_WAIT);
    CU_ASSERT_EQUAL(data->fd, 0);
    free_client_data(&data);
    CU_ASSERT_EQUAL(data, NULL);
}

MYTEST(socket_accept_incorrect)
{
    struct cs_data_t* data = accept_client_socket(0);
    CU_ASSERT_EQUAL(data, NULL);
}

MYTEST(socket_clients_close_correct)
{
    struct cs_node_t* head = NULL;
    int count = close_client_sockets_by_state(&head, SOCKET_NOSTATE);
    CU_ASSERT_EQUAL(count, 0);
}

MYTEST(socket_send_data_incorrect)
{
    int res = send_data(0, "\n", 1, 0);
    CU_ASSERT_EQUAL(res, DATA_NOT_SEND);
}

MYTEST(socket_recv_data_incorrect)
{
    char* bf = (char*) malloc(100);
    int res = recv_data(0, bf, sizeof(bf), 0);
    CU_ASSERT_EQUAL(res, DATA_NOT_SEND);
}