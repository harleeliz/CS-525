/************************************************************
 * File name:      test_helper.h
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This header file provides helper functions and macros for
 *   unit testing within the Record Manager project. It includes
 *   utility macros to check function return codes, compare
 *   strings and integers, assert boolean expressions, and mark
 *   test completion. The test framework uses these macros to
 *   report errors and successes in a standardized format.
 ************************************************************/

#ifndef TEST_HELPER_H
#define TEST_HELPER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "dberror.h"  // For RC_OK and errorMessage()

// Global variable to store the current test's name
extern char *testName;

// Shortcut for test information in log messages.
// This macro expands to include the current file, test name, line number, and time.
#define TEST_INFO  __FILE__, testName, __LINE__, __TIME__

/*
 * Macro: TEST_CHECK
 * -----------------
 * Checks the return code of a function call. If the code is not RC_OK,
 * it prints an error message with test information and exits the test.
 */
#define TEST_CHECK(code)                         \
		do {                                    \
			int rc_internal = (code);            \
			if (rc_internal != RC_OK) {          \
				char *message = errorMessage(rc_internal); \
				printf("[%s-%s-L%i-%s] FAILED: Operation returned error: %s\n", \
				       TEST_INFO, message);    \
				free(message);                 \
				exit(1);                       \
			}                                  \
		} while(0)

/*
 * Macro: ASSERT_EQUALS_STRING
 * ---------------------------
 * Checks whether two strings are equal. If they are not, it prints an error
 * message including the expected and actual strings, then exits the test.
 * If they are equal, a success message is printed.
 */
#define ASSERT_EQUALS_STRING(expected, real, message)        \
		do {                                               \
			if (strcmp((expected), (real)) != 0) {         \
				printf("[%s-%s-L%i-%s] FAILED: expected <%s> but was <%s>: %s\n", \
				       TEST_INFO, expected, real, message); \
				exit(1);                                 \
			}                                              \
			printf("[%s-%s-L%i-%s] OK: expected <%s> and was <%s>: %s\n", \
			       TEST_INFO, expected, real, message);  \
		} while(0)

/*
 * Macro: ASSERT_EQUALS_INT
 * ------------------------
 * Checks whether two integers are equal. If they are not, it prints an error
 * message with expected and actual values, then exits the test.
 * If they are equal, a success message is printed.
 */
#define ASSERT_EQUALS_INT(expected, real, message)         \
		do {                                           \
			if ((expected) != (real)) {              \
				printf("[%s-%s-L%i-%s] FAILED: expected <%i> but was <%i>: %s\n", \
				       TEST_INFO, expected, real, message); \
				exit(1);                             \
			}                                          \
			printf("[%s-%s-L%i-%s] OK: expected <%i> and was <%i>: %s\n", \
			       TEST_INFO, expected, real, message);  \
		} while(0)

/*
 * Macro: ASSERT_TRUE
 * ------------------
 * Checks whether a boolean expression is true. If not, it prints an error message
 * and exits the test. If the expression is true, a success message is printed.
 */
#define ASSERT_TRUE(real, message)                        \
		do {                                           \
			if (!(real)) {                           \
				printf("[%s-%s-L%i-%s] FAILED: expected true: %s\n", \
				       TEST_INFO, message);          \
				exit(1);                             \
			}                                          \
			printf("[%s-%s-L%i-%s] OK: expected true: %s\n", \
			       TEST_INFO, message);              \
		} while(0)

/*
 * Macro: ASSERT_ERROR
 * -------------------
 * Checks that a method call returns an error code (i.e., not RC_OK). If the call
 * returns RC_OK, it prints an error message and exits the test. If an error is
 * returned, it prints a success message with the error code.
 */
#define ASSERT_ERROR(expected, message)                    \
		do {                                           \
			int result = (expected);                 \
			if (result == RC_OK) {                   \
				printf("[%s-%s-L%i-%s] FAILED: expected an error: %s\n", \
				       TEST_INFO, message);          \
				exit(1);                             \
			}                                          \
			printf("[%s-%s-L%i-%s] OK: expected an error and was RC <%i>: %s\n", \
			       TEST_INFO, result, message);      \
		} while(0)

/*
 * Macro: TEST_DONE
 * ----------------
 * Marks the completion of a test. Prints a message indicating that the test has
 * finished successfully.
 */
#define TEST_DONE()                                         \
		do {                                            \
			printf("[%s-%s-L%i-%s] OK: finished test\n\n", TEST_INFO); \
		} while (0)

#endif // TEST_HELPER_H