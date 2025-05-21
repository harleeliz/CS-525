/************************************************************
 *     File name:                test_expr.c
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tables.h"
#include "expr.h"
#include "test_helper.h"
#include "dberror.h"

#undef TEST_CHECK

// Undefine MAKE_BOOL_VALUE if already defined
#ifdef MAKE_BOOL_VALUE
#undef MAKE_BOOL_VALUE
#endif

#define MAKE_BOOL_VALUE(result)                     \
    do {                                            \
        (result) = (Value *) calloc(1, sizeof(Value));\
        (result)->dt = DT_BOOL;                     \
    } while(0)

#define TEST_CHECK(code)                                          \
do {                                                            \
    int rc_internal = (code);                                     \
    if (rc_internal != RC_OK) {                                   \
        char *msg = errorMessage(rc_internal); /* no print usage */ \
        (void)msg; /* silence the unused-variable warning */        \
        printf("[%s-L%i-%s] FAILED: Operation returned rc=%d\n",    \
               __FILE__, __LINE__, __TIME__, rc_internal);          \
        exit(1);                                                    \
    }                                                             \
} while(0)

// Helper macros for operator tests
#define OP_TRUE(left, right, op, message)          \
    do {                                           \
        Value *result;                             \
        MAKE_INT_VALUE(result, -1);                \
        op(left, right, result);                   \
        bool b = result->v.boolV;                  \
        free(result);                              \
        ASSERT_TRUE(b, message);                   \
    } while (0)

#define OP_FALSE(left, right, op, message)         \
    do {                                           \
        Value *result;                             \
        MAKE_BOOL_VALUE(result);                   \
        op(left, right, result);                   \
        bool b = result->v.boolV;                  \
        free(result);                              \
        ASSERT_TRUE(!b, message);                  \
    } while (0)

/* Forward declarations for test functions */
static void testValueSerialize(void);
static void testOperators(void);
// static void testExpressions(void); // Uncomment to test complex expressions

/* Global variable for test name */
char *testName;

int main(void) {
    testName = "";
    testValueSerialize();
    testOperators();
    // testExpressions(); // Uncomment if testExpressions is implemented
    return 0;
}

/*
 * Test the serialization of Values.
 */
void testValueSerialize(void) {
    testName = "test value serialization and deserialization";
    ASSERT_EQUALS_STRING(serializeValue(stringToValue("i10")), "10", "create Value 10");
    ASSERT_EQUALS_STRING(serializeValue(stringToValue("f5.3")), "5.300000", "create Value 5.3");
    ASSERT_EQUALS_STRING(serializeValue(stringToValue("sHello World")), "Hello World", "create Value Hello World");
    ASSERT_EQUALS_STRING(serializeValue(stringToValue("bt")), "true", "create Value true");
    ASSERT_EQUALS_STRING(serializeValue(stringToValue("btrue")), "true", "create Value true");
    TEST_DONE();
}

/*
 * Test the basic operators: equality, comparison, and boolean operations.
 */
void testOperators(void) {
    Value *result;
    testName = "test value comparison and boolean operators";
    MAKE_INT_VALUE(result, 0);

    // Test equality
    OP_TRUE(stringToValue("i10"), stringToValue("i10"), valueEquals, "10 = 10");
    OP_FALSE(stringToValue("i9"), stringToValue("i10"), valueEquals, "9 != 10");

    OP_TRUE(stringToValue("sHello World"), stringToValue("sHello World"), valueEquals, "Hello World = Hello World");
    OP_FALSE(stringToValue("sHello Worl"), stringToValue("sHello World"), valueEquals, "Hello Worl != Hello World");
    OP_FALSE(stringToValue("sHello Worl"), stringToValue("sHello Wor"), valueEquals, "Hello Worl != Hello Wor");

    // Test smaller comparison
    OP_TRUE(stringToValue("i3"), stringToValue("i10"), valueSmaller, "3 < 10");
    OP_TRUE(stringToValue("f5.0"), stringToValue("f6.5"), valueSmaller, "5.0 < 6.5");

    // Test boolean operations
    OP_TRUE(stringToValue("bt"), stringToValue("bt"), boolAnd, "true AND true = true");
    OP_FALSE(stringToValue("bt"), stringToValue("bf"), boolAnd, "true AND false = false");
    OP_TRUE(stringToValue("bt"), stringToValue("bf"), boolOr, "true OR false = true");
    OP_FALSE(stringToValue("bf"), stringToValue("bf"), boolOr, "false OR false = false");

    TEST_CHECK(boolNot(stringToValue("bf"), result));
    ASSERT_TRUE(result->v.boolV, "!false = true");
    free(result);

    TEST_DONE();
}

/*
 // Uncomment and implement testExpressions if needed for complex expression tests.
 void testExpressions(void) {
     // Complex expression tests would go here.
     TEST_DONE();
 }
*/
