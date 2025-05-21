/************************************************************
 *     File name:                dberror.c
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dberror.h"
#include "config.h"

/* Global variable to hold the current error message */
char *RC_message = NULL;

/*
 * Returns a dynamically allocated string describing the error.
 * The caller is responsible for freeing the returned string.
 */
char *errorMessage(RC error) {
    char *message = NULL;
    switch(error) {
        case RC_OK:
            message = strdup("Success");
            break;
        case RC_ERROR:
            message = strdup("General error");
            break;
        case RC_FILE_NOT_FOUND:
            message = strdup("File not found");
            break;
        case RC_FILE_HANDLE_NOT_INIT:
            message = strdup("File handle not initialized");
            break;
        case RC_WRITE_FAILED:
            message = strdup("Write failed");
            break;
        case RC_READ_NON_EXISTING_PAGE:
            message = strdup("Attempt to read a non-existing page");
            break;
        /* Buffer Manager Errors */
        case RC_MALLOC_FAILED:
            message = strdup("Memory allocation failed");
            break;
        case RC_PINNED_PAGES_IN_BUFFER:
            message = strdup("Cannot shutdown due to pinned pages");
            break;
        case RC_BUFFER_POOL_NOT_INIT:
            message = strdup("Buffer pool not initialized");
            break;
        case RC_PAGE_NOT_FOUND:
            message = strdup("Page not found in buffer pool");
            break;
        case RC_NO_FREE_BUFFER_ERROR:
            message = strdup("No free buffer available");
            break;
        case RC_NO_AVAILABLE_FRAME:
            message = strdup("No available frame in buffer pool");
            break;
        /* Relation Manager Errors */
        case RC_RM_COMPARE_VALUE_OF_DIFFERENT_DATATYPE:
            message = strdup("Comparison of values of different datatypes");
            break;
        case RC_RM_EXPR_RESULT_IS_NOT_BOOLEAN:
            message = strdup("Expression result is not boolean");
            break;
        case RC_RM_BOOLEAN_EXPR_ARG_IS_NOT_BOOLEAN:
            message = strdup("Boolean expression argument is not boolean");
            break;
        case RC_RM_NO_MORE_TUPLES:
            message = strdup("No more tuples");
            break;
        case RC_RM_NO_PRINT_FOR_DATATYPE:
            message = strdup("No print function for this datatype");
            break;
        case RC_RM_UNKNOWN_DATATYPE:
            message = strdup("Unknown datatype");
            break;
        case RC_RM_EXPR_NOT_SUPPORTED:
            message = strdup("Expression not supported");
            break;
        case RC_RM_INVALID_ATTR_NUM:
            message = strdup("Invalid attribute number");
            break;
        case RC_RM_INVALID_DATATYPE:
            message = strdup("Invalid datatype");
            break;
        case RC_INVALID_RID:
            message = strdup("Invalid record ID");
            break;
        /* Index Manager Errors */
        case RC_IM_KEY_NOT_FOUND:
            message = strdup("Key not found in index");
            break;
        case RC_IM_KEY_ALREADY_EXISTS:
            message = strdup("Key already exists in index");
            break;
        case RC_IM_N_TO_LARGE:
            message = strdup("Value too large");
            break;
        case RC_IM_NO_MORE_ENTRIES:
            message = strdup("No more entries in index");
            break;
        /* Table & Schema Errors */
        case TABLE_DOES_NOT_EXIST:
            message = strdup("Table does not exist");
            break;
        case RC_PARAMS_ERROR:
            message = strdup("Invalid function parameters");
            break;
        case RC_TABLE_EXISTS:
            message = strdup("Table already exists");
            break;
        case RC_TABLE_CREATES_FAILED:
            message = strdup("Table creation failed");
            break;
        case RC_SCHEMA_TOO_LARGE:
            message = strdup("Schema exceeds page size limit");
            break;
        default:
            message = strdup("Unknown error code");
            break;
    }
    return message;
}

/*
 * Prints the error message corresponding to the given error code.
 */
void printError(RC error) {
    char *msg = errorMessage(error);
    fprintf(stderr, "Error: %s\n", msg);
    free(msg);
}
