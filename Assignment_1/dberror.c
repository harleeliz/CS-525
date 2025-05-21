/************************************************************
*     File name:                dberror.c                   *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/

#include "dberror.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *RC_message;

// Print a message to standard out describing the error
void printError(RC error) {
    if (RC_message != NULL) {
        printf("Error Code (%i): \"%s\"\n", error, RC_message);
    } else {
        printf("Error Code (%i): No additional message provided.\n", error);
    }
}

// Retrieve an error message string corresponding to the given error code
char *errorMessage(RC error) {
    char *message;
    const char *baseMessage;

    // Define generic messages for common return codes
    switch (error) {
        case RC_OK:
            baseMessage = "Operation successful.";
            break;
        case RC_FILE_NOT_FOUND:
            baseMessage = "File not found.";
            break;
        case RC_FILE_HANDLE_NOT_INIT:
            baseMessage = "File handle not initialized.";
            break;
        case RC_WRITE_FAILED:
            baseMessage = "Write operation failed.";
            break;
        case RC_READ_NON_EXISTING_PAGE:
            baseMessage = "Attempted to read a non-existing page.";
            break;
        default:
            baseMessage = "Unknown error.";
    }

    // If RC_message exists, combine it with the base message
    if (RC_message != NULL) {
        message = (char *) malloc(strlen(baseMessage) + strlen(RC_message) + 50);
        sprintf(message, "Error Code (%i): %s Additional Info: %s", error, baseMessage, RC_message);
    } else {
        message = (char *) malloc(strlen(baseMessage) + 30);
        sprintf(message, "Error Code (%i): %s", error, baseMessage);
    }

    return message;
}

// Utility function to set the global error message
void setErrorMessage(const char *message) {
    if (RC_message != NULL) {
        free(RC_message); // Free previously allocated memory
    }
    RC_message = strdup(message); // Duplicate the new message
}

// Cleanup function to clear error messages
void clearErrorMessage() {
    if (RC_message != NULL) {
        free(RC_message);
        RC_message = NULL;
    }
}
