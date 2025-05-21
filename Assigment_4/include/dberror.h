/************************************************************
 *     File name:                dberror.h
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/

#ifndef DBERROR_H
#define DBERROR_H
#pragma once

/* ------------------------------- */
/*         Module Constants        */
/* ------------------------------- */
#define PAGE_SIZE 4096  /* Page size in bytes */

/* RC (Return Code) is defined as an integer */
typedef int RC;

/* ------------------------------- */
/*      General System Errors      */
/* ------------------------------- */
#define RC_OK                           0     /* Success */
#define RC_ERROR                        -500  /* General error */
#define RC_FILE_NOT_FOUND               -1    /* File not found */
#define RC_FILE_HANDLE_NOT_INIT         -2    /* File handle not initialized */
#define RC_WRITE_FAILED                 -3    /* Write failed */
#define RC_READ_NON_EXISTING_PAGE       -4    /* Attempt to read a non-existent page */

/* ------------------------------- */
/*       Buffer Manager Errors     */
/* ------------------------------- */
#define RC_MALLOC_FAILED                -1000 /* Memory allocation failed */
#define RC_PINNED_PAGES_IN_BUFFER       -1001 /* Cannot shut down due to pinned pages */
#define RC_BUFFER_POOL_NOT_INIT         -1002 /* Buffer pool not initialized */
#define RC_PAGE_NOT_FOUND               -1003 /* Page not found in buffer pool */
#define RC_NO_FREE_BUFFER_ERROR         -1004 /* No free buffer available */
#define RC_NO_AVAILABLE_FRAME           -1005 /* No available frame in buffer pool */

/* ------------------------------- */
/*      Relation Manager Errors    */
/* ------------------------------- */
#define RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE -200  /* Data type mismatch */
#define RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN          -201  /* Expression result is not boolean */
#define RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN     -202  /* Boolean expression argument is not boolean */
#define RC_RM_NO_MORE_TUPLES                      -203  /* No more tuples to process */
#define RC_RM_NO_PRINT_FOR_DATATYPE               -204  /* Cannot print this data type */
#define RC_RM_UNKNOWN_DATATYPE                    -205  /* Unknown data type */
#define RC_RM_EXPR_NOT_SUPPORTED                  -206  /* Expression operation not supported */
#define RC_RM_INVALID_ATTR_NUM                    -502  /* Invalid attribute number */
#define RC_RM_INVALID_DATATYPE                    -503  /* Invalid data type */
#define RC_INVALID_RID                            -104  /* Invalid record ID */

/* ------------------------------- */
/*      Index Manager Errors       */
/* ------------------------------- */
#define RC_IM_KEY_NOT_FOUND       -300  /* Key not found in index */
#define RC_IM_KEY_ALREADY_EXISTS  -301  /* Key already exists */
#define RC_IM_N_TO_LARGE          -302  /* Value too large */
#define RC_IM_NO_MORE_ENTRIES     -303  /* No more entries */

/* ------------------------------- */
/*     Table & Schema Errors       */
/* ------------------------------- */
#define TABLE_DOES_NOT_EXIST      -100  /* Table does not exist */
#define RC_PARAMS_ERROR           -101  /* Invalid function parameters */
#define RC_TABLE_EXISTS           -102  /* Table already exists */
#define RC_TABLE_CREATES_FAILED   -103  /* Table creation failed */
#define RC_INVALID_RID            -104          /* Invalid record ID */
#define RC_SCHEMA_TOO_LARGE       -105     /* Schema exceeds page size limit */



/* ------------------------------- */
/*       Error Handling API        */
/* ------------------------------- */

/* Global variable to hold error messages */
extern char *RC_message;

/* Prints the error message associated with the given error code */
void printError(RC error);

/* Returns a dynamically allocated string describing the error.
   The caller is responsible for freeing the returned string. */
char *errorMessage(RC error);

/* Macro to throw an error with a message */
#define THROW(rc, message)               \
do {                                     \
    RC_message = message;                \
    return rc;                           \
} while (0)

/* Macro to check the return code and exit if an error occurs */
#define CHECK(code)                                                      \
do {                                                                     \
    int rc_internal = (code);                                            \
    if (rc_internal != RC_OK) {                                          \
        char *message = errorMessage(rc_internal);                       \
        printf("[%s-L%i-%s] ERROR: %s\n", __FILE__, __LINE__, __TIME__, message); \
        free(message);                                                   \
        exit(1);                                                         \
    }                                                                    \
} while (0)

#endif // DBERROR_H
