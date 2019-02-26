#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

#define TEST(name) \
    static void test_##name()

#define ADD_SUITE_TEST(suite, name) \
    if ((NULL = CU_add_suite(suite, #name, (CU_TestFunc)test_##name))) { \
        CU_cleanup_registry(); \
    }

static int CU_myinit_suite(void);
static int CU_myclear_suite(void);

#endif // !__UTILS_H__