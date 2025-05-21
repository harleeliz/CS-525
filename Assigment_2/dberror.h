/************************************************************
*     File name:                    dberror.h               *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/

#ifndef DBERROR_H  // Include guard to prevent multiple inclusions of this header file
#define DBERROR_H

/* Module-wide constants */
#define PAGE_SIZE 4096 // Define PAGE_SIZE as 4096 bytes

/* Return code definitions (RC is used to represent the result of a function call) */
typedef int RC; // Define RC as an integer type

/* ============================ */
/* General / File System Errors */
/* ============================ */
#define RC_OK                          0    // The operation was successful
#define RC_FILE_NOT_FOUND              1    // File not found
#define RC_FILE_HANDLE_NOT_INIT        2    // File handle isn't initialized
#define RC_WRITE_FAILED                3    // Write operation failed
#define RC_READ_NON_EXISTING_PAGE      4    // Attempted to read a non-existent page

/* ====================== */
/* Buffer Manager Errors  */
/* ====================== */
#define RC_MALLOC_FAILED             1000   // Memory allocation failed
#define RC_PINNED_PAGES_IN_BUFFER    1001   // Attempt to shut down with pinned pages in buffer pool
#define RC_BUFFER_POOL_NOT_INIT      1002   // Buffer pool not initialized
#define RC_PAGE_NOT_FOUND            1003   // The page wasn't found in the buffer pool
#define RC_NO_FREE_BUFFER_ERROR      1004   // No free buffer available
#define RC_NO_AVAILABLE_FRAME        1005   // No available frame in the buffer pool

/* ============================ */
/* Relation Manager Errors      */
/* ============================ */
#define RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE 200   // Relation Manager: Comparison of different data types
#define RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN          201   // Relation Manager: Expression result is not boolean
#define RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN     202   // Relation Manager: Boolean expression argument is not boolean
#define RC_RM_NO_MORE_TUPLES                      203   // Relation Manager: No more tuples to process
#define RC_RM_NO_PRINT_FOR_DATATYPE               204   // Relation Manager: Cannot print this data type
#define RC_RM_UNKNOWN_DATATYPE                    205   // Relation Manager: Unknown data type

/* ============================ */
/* Index Manager Errors         */
/* ============================ */
#define RC_IM_KEY_NOT_FOUND      300   // Index Manager: Key not found
#define RC_IM_KEY_ALREADY_EXISTS 301   // Index Manager: Key already exists
#define RC_IM_N_TO_LAGE          302   // Index Manager: Value too large
#define RC_IM_NO_MORE_ENTRIES    303   // Index Manager: No more entries


/* Holder for error messages (Global variable to store the error message) */
extern char *RC_message;

/* Print a message to standard output describing the error */
extern void printError(RC error); // Function declaration to print error message
extern char *errorMessage(RC error); // Function declaration to get error message string

/* Macro for throwing an error (Simplifies error handling) */
#define THROW(rc,message) \
do {           \
    RC_message=message;      \
    return rc;      \
} while (0)
/* Check the return code and exit if it is an error (For debugging and testing) */
#define CHECK(code)                   \
do {                           \
    int rc_internal = (code);                 \
    if (rc_internal != RC_OK)                 \
    {                          \
        char *message = errorMessage(rc_internal);       \
        printf("[%s-L%i-%s] ERROR: Operation returned error: %s\n",__FILE__, __LINE__, __TIME__, message); \
        free(message);                   \
        exit(1);                     \
    }                          \
} while(0);

#endif // End of include guard
