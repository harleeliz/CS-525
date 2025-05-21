/************************************************************
*     File name:                test_helper.h               *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/

#ifndef TEST_HELPER_H  // Include guard to prevent multiple inclusions
#define TEST_HELPER_H

// Variable to store the current test's name.  This is likely used for informative output.
extern char *testName;

// Shortcut for test information.  This macro combines file name, test name, line number, and time.
#define TEST_INFO  __FILE__, testName, __LINE__, __TIME__

// Check the return code and exit if it's an error.  This macro simplifies error checking in tests.
#define TEST_CHECK(code)                   \
       do {                           \
          int rc_internal = (code);                 \
          if (rc_internal != RC_OK)                 \
          {                          \
             char *message = errorMessage(rc_internal);       \
             printf("[%s-%s-L%i-%s] FAILED: Operation returned error: %s\n",TEST_INFO, message); \
             free(message);                   \
             exit(1);                     \
          }                          \
       } while(0);

// Check whether two strings are equal.  This macro compares two strings and prints a success/failure message.
#define ASSERT_EQUALS_STRING(expected,real,message)       \
       do {                           \
          if (strcmp((expected),(real)) != 0)                \
          {                          \
             printf("[%s-%s-L%i-%s] FAILED: expected <%s> but was <%s>: %s\n",TEST_INFO, expected, real, message); \
             exit(1);                     \
          }                          \
          printf("[%s-%s-L%i-%s] OK: expected <%s> and was <%s>: %s\n",TEST_INFO, expected, real, message); \
       } while(0);

// Check whether two ints are equal.  This macro compares two integers and prints a success/failure message.
#define ASSERT_EQUALS_INT(expected,real,message)          \
       do {                           \
          if ((expected) != (real))              \
          {                          \
             printf("[%s-%s-L%i-%s] FAILED: expected <%i> but was <%i>: %s\n",TEST_INFO, expected, real, message); \
             exit(1);                     \
          }                          \
          printf("[%s-%s-L%i-%s] OK: expected <%i> and was <%i>: %s\n",TEST_INFO, expected, real, message); \
       } while(0);

// Check whether a condition is true.  This macro asserts that a given expression evaluates to true.
#define ASSERT_TRUE(real,message)               \
       do {                           \
          if (!(real))                     \
          {                          \
             printf("[%s-%s-L%i-%s] FAILED: expected true: %s\n",TEST_INFO, message); \
             exit(1);                     \
          }                          \
          printf("[%s-%s-L%i-%s] OK: expected true: %s\n",TEST_INFO, message); \
       } while(0);

// Check that a method returns an error code.  This macro verifies that a function call returns a specific error code.
#define ASSERT_ERROR(expected,message)     \
       do {                           \
          int result = (expected);                  \
          if (result == (RC_OK))                \
          {                          \
             printf("[%s-%s-L%i-%s] FAILED: expected an error: %s\n",TEST_INFO, message); \
             exit(1);                     \
          }                          \
          printf("[%s-%s-L%i-%s] OK: expected an error and was RC <%i>: %s\n",TEST_INFO,  result , message); \
       } while(0);

// Test worked.  This macro prints a message indicating that a test has finished successfully.
#define TEST_DONE()                   \
       do {                           \
          printf("[%s-%s-L%i-%s] OK: finished test\n\n",TEST_INFO); \
       } while (0);

#endif // TEST_HELPER_H
