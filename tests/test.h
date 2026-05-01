#pragma once

#ifndef TEST_PRINT_PAD
#define TEST_PRINT_PAD -64
#endif

#define TEST_PASS_STR "\33[1;32mPASS\33[0m"
#define TEST_FAIL_STR "\33[1;31mFAIL\33[0m"

#ifdef TEST_IMPLEMENTATION

#include <stdio.h>
#include <stdbool.h>

#define TEST(test_name) static void test_##test_name(bool *test_failed)

#define TEST_ASSERT(condition)                                      \
    do {                                                            \
        if (!(condition)) {                                         \
            *test_failed = true;                                    \
            printf("\33[0;31mAssertion failed: %s (%s:%d)\33[0m\n", \
                   #condition, __FILE__, __LINE__);                 \
            return;                                                 \
        }                                                           \
    } while (0)

#define TEST_ASSERT_NULL(ptr) TEST_ASSERT(ptr == NULL)
#define TEST_ASSERT_NOT_NULL(ptr) TEST_ASSERT(ptr != NULL)

static bool test_failed;
static int tests_run = 0, tests_passed = 0;

static void test_run(void (*test)(bool *), const char *test_name) {
    test_failed = false;
    test(&test_failed);
    tests_run++;
    printf("%*s", TEST_PRINT_PAD, test_name);
    if (!test_failed) {
        puts(TEST_PASS_STR);
        tests_passed++;
    } else {
        puts(TEST_FAIL_STR);
    }
}

#define TEST_RUN(test_name) test_run(test_##test_name, #test_name);

#define TEST_CONCLUDE \
    printf("\033[1m[%d/%d tests passed]\33[0m\n", tests_passed, tests_run); \
    return tests_passed == tests_run ? 0 : 1

#endif // TEST_IMPLEMENTATION
