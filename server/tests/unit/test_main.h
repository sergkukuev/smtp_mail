#ifndef __TEST_MAIN_H__
#define __TEST_MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

#define TEST(name) \
    static void test_##name()

#define ADD_SUITE_TEST(suite, name) \
    if ((NULL = CU_add_suite(suite, #name, (CU_TestFunc)test_##name))) { \
        CU_cleanup_registry(); \
    }

int add_mytests(CU_pSuite suite);

#endif