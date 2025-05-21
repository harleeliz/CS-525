/************************************************************
*     File name:                test_helper.h
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/

#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <stdio.h>
#include "dberror.h"

// Global variable for test name (set in your test files)
extern char *testName;

// Macro to include test information (file, testName, line, time)
#define TEST_INFO __FILE__, testName, __LINE__, __TIME__

// Macro to check a function's return code and exit if an error occurs.
#define TEST_CHECK(code)                                                   \
    do {                                                                   \
        int rc_internal = (code);                                          \
        if (rc_internal != RC_OK) {                                        \
            char *message = errorMessage(rc_internal); /* may be unused */   \
            /* Print the test information along with error code and message */ \
            printf("[%s - %s - L%i - %s] FAILED: Operation returned error code: %d (%s)\n", \
                   __FILE__, testName, __LINE__, __TIME__, rc_internal, message); \
            exit(1);                                                       \
        }                                                                  \
    } while (0)

// Macro to assert that two strings are equal.
#define ASSERT_EQUALS_STRING(expected, real, message)                      \
    do {                                                                   \
        if (strcmp((expected), (real)) != 0) {                              \
            printf("[%s - %s - L%i - %s] FAILED: expected <%s> but was <%s>: %s\n", \
                   __FILE__, testName, __LINE__, __TIME__, (expected), (real), (message)); \
            exit(1);                                                       \
        } else {                                                           \
            printf("[%s - %s - L%i - %s] OK: expected <%s> and was <%s>: %s\n", \
                   __FILE__, testName, __LINE__, __TIME__, (expected), (real), (message)); \
        }                                                                  \
    } while (0)

// Macro to assert that two integers are equal.
#define ASSERT_EQUALS_INT(expected, real, message)                         \
    do {                                                                   \
        if ((expected) != (real)) {                                        \
            printf("[%s - %s - L%i - %s] FAILED: expected <%d> but was <%d>: %s\n", \
                   __FILE__, testName, __LINE__, __TIME__, (expected), (real), (message)); \
            exit(1);                                                       \
        } else {                                                           \
            printf("[%s - %s - L%i - %s] OK: expected <%d> and was <%d>: %s\n", \
                   __FILE__, testName, __LINE__, __TIME__, (expected), (real), (message)); \
        }                                                                  \
    } while (0)

// Macro to assert that a condition is true.
#define ASSERT_TRUE(condition, message)                                    \
    do {                                                                   \
        if (!(condition)) {                                                \
            printf("[%s - %s - L%i - %s] FAILED: expected true: %s\n",       \
                   __FILE__, testName, __LINE__, __TIME__, (message));       \
            exit(1);                                                       \
        } else {                                                           \
            printf("[%s - %s - L%i - %s] OK: condition is true: %s\n",        \
                   __FILE__, testName, __LINE__, __TIME__, (message));       \
        }                                                                  \
    } while (0)

// Macro to assert that an error is returned.
#define ASSERT_ERROR(expected, message)                                    \
    do {                                                                   \
        int result = (expected);                                           \
        if (result == RC_OK) {                                             \
            printf("[%s - %s - L%i - %s] FAILED: expected an error: %s\n",    \
                   __FILE__, testName, __LINE__, __TIME__, (message));       \
            exit(1);                                                       \
        } else {                                                           \
            printf("[%s - %s - L%i - %s] OK: expected an error and got error code <%d>: %s\n", \
                   __FILE__, testName, __LINE__, __TIME__, result, (message)); \
        }                                                                  \
    } while (0)

// Macro to indicate that the test has completed successfully.
#define TEST_DONE()                                                          \
    do {                                                                   \
        printf("[%s - %s - L%i - %s] OK: finished test\n\n",                \
               __FILE__, testName, __LINE__, __TIME__);                      \
    } while (0)

#endif /* TEST_HELPER_H */
