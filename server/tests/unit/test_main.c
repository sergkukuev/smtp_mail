#include "test_main.h"
#include "utils.h"

// know that there is better not to do
#include "test_handlers.c"
#include "test_sockets.c"

/*TEST(full_session_correct)
{
}*/

int add_mytests(CU_pSuite suite)
{
    if (suite) {
        // add tests here
        // sockets
        ADD_SUITE_TEST(suite, socket_listen_correct)
        //ADD_SUITE_TEST(suite, socket_bind_client_data_correct)
        ADD_SUITE_TEST(suite, socket_accept_incorrect)
        ADD_SUITE_TEST(suite, socket_clients_close_correct)
        ADD_SUITE_TEST(suite, socket_send_data_incorrect)
        ADD_SUITE_TEST(suite, socket_recv_data_incorrect)
        // handlers
        //ADD_SUITE_TEST(suite, full_correct_session)
        ADD_SUITE_TEST(suite, handle_helo_correct)
        ADD_SUITE_TEST(suite, handle_helo_incorrect)
        ADD_SUITE_TEST(suite, handle_ehlo_correct)
        ADD_SUITE_TEST(suite, handle_ehlo_incorrect)
        ADD_SUITE_TEST(suite, handle_mail_correct)
        ADD_SUITE_TEST(suite, handle_mail_incorrect_state)
        ADD_SUITE_TEST(suite, handle_mail_incorrect)
        ADD_SUITE_TEST(suite, handle_rcpt_correct)
        ADD_SUITE_TEST(suite, handle_rcpt_incorrect_state)
        ADD_SUITE_TEST(suite, handle_rcpt_incorrect)
        ADD_SUITE_TEST(suite, handle_data_correct)
        ADD_SUITE_TEST(suite, handle_data_incorrect_state)
        ADD_SUITE_TEST(suite, handle_data_incorrect)
        ADD_SUITE_TEST(suite, handle_rset_correct)
        ADD_SUITE_TEST(suite, handle_rset_incorrect)
        ADD_SUITE_TEST(suite, handle_vrfy_correct)
        ADD_SUITE_TEST(suite, handle_vrfy_incorrect)
        ADD_SUITE_TEST(suite, handle_quit_correct)
        ADD_SUITE_TEST(suite, handle_quit_incorrect)
        ADD_SUITE_TEST(suite, handle_noop_correct)
        ADD_SUITE_TEST(suite, handle_noop_incorrect)
        ADD_SUITE_TEST(suite, handle_undef_correct)
        ADD_SUITE_TEST(suite, handle_undef_incorrect)
    }
    return CU_get_error();
}

