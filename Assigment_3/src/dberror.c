/************************************************************
 * File name:      dberror.c
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This source file implements error handling functions for the
 *   Record Manager project. It defines a global error message
 *   variable and provides functions to print error messages and
 *   to retrieve a copy of the current error message.
 ************************************************************/

#include "dberror.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Global variable to store the current error message
char *RC_message = NULL;

/*
 * Function: printError
 * --------------------
 * Prints an error message to standard output.
 *
 * If RC_message is set, it prints the error code along with the message.
 * Otherwise, it prints only the error code.
 *
 * Parameters:
 *   error - The error code to be printed.
 */
void printError(RC error)
{
    if (RC_message != NULL)
        printf("EC (%i), \"%s\"\n", error, RC_message);
    else
        printf("EC (%i)\n", error);
}

/*
 * Function: errorMessage
 * ----------------------
 * Returns a dynamically allocated copy of the current error message.
 *
 * If RC_message is not set, a default message is returned.
 *
 * Parameters:
 *   error - The error code (not directly used in this implementation,
 *           but provided for consistency with the interface).
 *
 * Returns:
 *   A pointer to a dynamically allocated string containing the error message.
 *   The caller is responsible for freeing this string.
 */
char *errorMessage(RC error)
{
    char *message;
    if (RC_message != NULL)
    {
        // Allocate memory and copy the current error message
        message = (char *) malloc(strlen(RC_message) + 1);
        strcpy(message, RC_message);
    }
    else
    {
        // Provide a default error message if none is set
        message = (char *) malloc(32 * sizeof(char));
        strcpy(message, "No error message provided");
    }
    return message;
}