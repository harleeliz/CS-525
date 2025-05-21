/************************************************************
*     File name:                    dberror.c               *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/

#include "dberror.h" // Include header file for error codes and messages
#include <string.h>  // Include string manipulation functions
#include <stdlib.h>  // Include standard library functions (malloc, sprintf)
#include <stdio.h>   // Include standard input/output functions (printf)

char *RC_message; // Global variable to store the error message string

/*
 * Prints an error message to standard output.
 *
 * @param error The error code (RC type).
 */
void printError(RC error) {
    // Check if a detailed error message is available
    if (RC_message != NULL)
        printf("EC (%i), \"%s\"\n", error, RC_message); // Print error code and message
    else
        printf("EC (%i)\n", error); // Print only the error code
}

/*
 * Generates an error message string.  The caller should free this string.
 *
 * @param error The error code (RC type).
 * @return A dynamically allocated string containing the error message.  The caller is responsible for freeing this memory.
 */
char *errorMessage(RC error) {
    char *message; // Pointer to store the error message

    // Check if a detailed error message is available
    if (RC_message != NULL) {
        // Allocate memory for the error message string (including space for error code and message)
        message = (char *) malloc(strlen(RC_message) + 30);
        // Format the error message string
        sprintf(message, "EC (%i), \"%s\"\n", error, RC_message);
    } else {
        // Allocate memory for the error message string (only error code)
        message = (char *) malloc(30);
        // Format the error message string (only error code)
        sprintf(message, "EC (%i)\n", error);
    }

    return message; // Return the error message string.  The Caller is responsible for freeing this memory.
}
