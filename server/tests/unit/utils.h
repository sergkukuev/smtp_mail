#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdlib.h>
#include <stdio.h>
#include <CUnit/Basic.h>

#define MYTEST(name) \
    static void test_##name()

#define ADD_SUITE_TEST(suite, name) \
    if ((NULL == CU_add_test(suite, #name, (CU_TestFunc)test_##name))) {\
        CU_cleanup_registry();\
    }

#endif