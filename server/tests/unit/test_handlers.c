#include "utils.h"
#include "../../src/handlers.h"
#include "../../src/sockets.h"

MYTEST(handle_helo_correct)
{
    char* msg = "HELO server.ru";
    int fd = init_listen_socket();
    struct cs_data_t* data = accept_client_socket(fd);

    int err = HELO_handle(data, msg);
    
    CU_ASSERT_EQUAL(data->fd, 0);
    CU_ASSERT_EQUAL(data->state, SOCKET_STATE_WAIT);
    CU_ASSERT_EQUAL(err, 0);
    free_client_data(&data);
    close_listen_socket(fd);
}