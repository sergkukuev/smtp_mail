#include "test_main.h"
#include "utils.h"

// know that there is better not to do
#include "test_handlers.c"

/*TEST(full_session_correct)
{
}*/

int add_mytests(CU_pSuite suite)
{
    if (suite) {
        // TODO: add tests here
        ADD_SUITE_TEST(suite, handle_helo_correct)
    }
    return CU_get_error();
}

