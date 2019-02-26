#include <stdlib.h>
#include <stdio.h>
#include <CUnit/Basic.h>

#include "unit/test_main.h"

/* need to install
libcunit1 libcunit1-doc libcunit1-dev
*/

// stubs for init and clear
static int CU_myinit_suite(void) 
{
    return 0;
}

static int CU_myclear_suite(void) 
{
    return 0;
}

CU_pSuite CU_mycreate_suite(const char* title)
{
    CU_pSuite suite = CU_add_suite(title, CU_myinit_suite, CU_myclear_suite);
    if (suite == NULL) {
        CU_cleanup_registry();
        printf("%s\n", CU_get_error_msg());
    }
    return suite;
}

int main(void) {
    if (CU_initialize_registry() != CUE_SUCCESS) {
        printf("%s\n", CU_get_error_msg());
        exit(CU_get_error());
    }

    CU_pSuite suite = CU_mycreate_suite("suite_test");
    (suite == NULL) ? exit(CU_get_error()) : add_mytests(suite);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    /*
    printf("\n================================LOG================================\n");
    CU_basic_show_failures(CU_get_failure_list());
    printf("\n\n================================END================================\n\n");
    */
}
