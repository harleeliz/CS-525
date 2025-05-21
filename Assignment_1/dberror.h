/************************************************************
*     File name:                dberror.h                   *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/
#ifndef DBERROR_H
#define DBERROR_H


// Module-wide constants
#define PAGE_SIZE 4096

// Return code definitions
typedef int RC;

#define RC_OK 0
#define RC_FILE_NOT_FOUND 1
#define RC_FILE_HANDLE_NOT_INIT 2
#define RC_WRITE_FAILED 3
#define RC_READ_NON_EXISTING_PAGE 4

#define RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE 200
#define RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN 201
#define RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN 202
#define RC_RM_NO_MORE_TUPLES 203
#define RC_RM_NO_PRINT_FOR_DATATYPE 204
#define RC_RM_UNKOWN_DATATYPE 205

#define RC_IM_KEY_NOT_FOUND 300
#define RC_IM_KEY_ALREADY_EXISTS 301
#define RC_IM_N_TO_LAGE 302
#define RC_IM_NO_MORE_ENTRIES 303

// Holder for error messages
extern char *RC_message;

// Print a message to standard out describing the error
extern void printError(RC error);

// Retrieve a detailed error message for a given error code
extern char *errorMessage(RC error);

// THROW Macro: Assigns an error message and returns the error code
#define THROW(rc, message)                 \
    do {                                   \
        RC_message = message;              \
        return rc;                         \
    } while (0)

// CHECK Macro: Verifies return codes and exits with an error message if needed
#define CHECK(code)                                                         \
    do {                                                                    \
        int rc_internal = (code);                                           \
        if (rc_internal != RC_OK) {                                         \
            char *message = errorMessage(rc_internal);                      \
            printf("[%s-L%i-%s] ERROR: Operation returned error: %s\n",     \
                   __FILE__, __LINE__, __TIME__, message);                  \
            free(message);                                                  \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

#endif // DBERROR_H
