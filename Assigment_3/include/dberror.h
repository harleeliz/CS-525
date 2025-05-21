/************************************************************
 * File name: dberror.h
 * Course: CS 525 Advanced Database Organization (Spring 2025)
 * Authors: Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This header file defines error codes and error handling
 *   macros used across the database module. It provides a
 *   unified way to return error codes and print corresponding
 *   error messages for various error conditions.
 ************************************************************/

#ifndef DBERROR_H
#define DBERROR_H

#include <stdio.h>  // Standard I/O functions

/* Module wide constant for the size of a database page */
#define PAGE_SIZE 4096

/*
 * Return code definitions:
 * RC is defined as an integer type for returning error codes from functions.
 */
typedef int RC;

/*------------------------------------------------------------
 * General error codes
 *-----------------------------------------------------------*/
#define RC_OK                                 0  // Operation completed successfully
#define RC_FILE_NOT_FOUND                     1  // File not found error
#define RC_FILE_HANDLE_NOT_INIT               2  // File handle has not been initialized
#define RC_WRITE_FAILED                       3  // Failed write operation
#define RC_READ_NON_EXISTING_PAGE             4  // Attempt to read a page that does not exist
#define RC_ERROR                              5  // Generic error code
#define RC_PARAMS_ERROR                       6  // Invalid parameters provided
#define RC_ALLOC_MEM_FAIL                     7  // Memory allocation failed
#define RC_DATATYPE_MISMATCH                  8  // Data type mismatch encountered
#define RC_DATATYPE_UNDEFINE                  9  // Undefined data type encountered

/*------------------------------------------------------------
 * Table-related error codes
 *-----------------------------------------------------------*/
#define RC_TABLE_NOT_EXISTS                  100  // Specified table does not exist
#define RC_TABLE_EXISTS                      101  // Specified table already exists
#define RC_TABLE_CREATES_FAILED              102  // Failed to create table
#define RC_NO_SCHEMA_DATA                    103  // Schema data is missing

/*------------------------------------------------------------
 * Record Manager error codes
 *-----------------------------------------------------------*/
#define RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE 200  // Comparison between different data types
#define RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN          201  // Expression did not evaluate to a boolean
#define RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN      202  // Argument in a boolean expression is not boolean
#define RC_RM_NO_MORE_TUPLES                        203  // No more tuples available in the record set
#define RC_RM_NO_PRINT_FOR_DATATYPE                 204  // No print function for the given data type
#define RC_RM_UNKOWN_DATATYPE                       205  // Encountered an unknown data type

/*------------------------------------------------------------
 * Index Manager error codes
 *-----------------------------------------------------------*/
#define RC_IM_KEY_NOT_FOUND                   300  // Specified key not found in the index
#define RC_IM_KEY_ALREADY_EXISTS              301  // Specified key already exists in the index
#define RC_IM_N_TO_LAGE                       302  // The provided 'N' value is too large (index-specific error)
#define RC_IM_NO_MORE_ENTRIES                 303  // No more entries available in the index

/*------------------------------------------------------------
 * Global variable to hold error messages.
 *-----------------------------------------------------------*/
extern char *RC_message;

/*
 * Function: printError
 * --------------------
 * Description:
 *   Prints a descriptive error message corresponding to the given error code.
 *
 * Parameters:
 *   error - The error code for which the message will be printed.
 */
extern void printError (RC error);

/*
 * Function: errorMessage
 * ----------------------
 * Description:
 *   Returns a descriptive error message string corresponding to the given error code.
 *
 * Parameters:
 *   error - The error code for which the message is needed.
 *
 * Returns:
 *   A pointer to a character string containing the error message.
 */
extern char *errorMessage (RC error);

/*
 * Macro: THROW
 * ------------
 * Description:
 *   Sets the global error message and returns an error code from the current function.
 *
 * Parameters:
 *   rc      - The error code to be returned.
 *   message - The error message to be set.
 */
#define THROW(rc, message)       \
    do {                         \
        RC_message = message;    \
        return rc;               \
    } while (0)

/*
 * Macro: CHECK
 * ------------
 * Description:
 *   Executes a code block, checks the returned error code, and if it is not RC_OK,
 *   prints a detailed error message (including file name, line number, and time),
 *   frees any allocated memory for the error message, and exits the program.
 *
 * Parameters:
 *   code - The operation (typically a function call) whose return code is to be checked.
 */
#define CHECK(code)                                                    \
    do {                                                               \
        int rc_internal = (code);                                      \
        if (rc_internal != RC_OK) {                                    \
            char *message = errorMessage(rc_internal);                 \
            printf("[%s-L%i-%s] ERROR: Operation returned error: %s\n", \
                   __FILE__, __LINE__, __TIME__, message);             \
            free(message);                                             \
            exit(1);                                                   \
        }                                                              \
    } while(0)

#endif // DBERROR_H