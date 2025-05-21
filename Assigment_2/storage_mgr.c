/************************************************************
*     File name:                storage_mgr.c               *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/

#include "storage_mgr.h" // Include header file for storage manager definitions
#include "dberror.h"    // Include header file for error codes and messages
#include <stdio.h>     // Include standard input/output functions
#include <stdlib.h>    // Include standard library functions (malloc, calloc, free)

/*
 * Initializes the storage manager.
 * Currently, this function only prints a message.  No actual initialization is performed.
 */
void initStorageManager(void) {
    printf("Storage Manager initialized.\n");
}

/*
 * Creates a new page file with one empty page.
 *
 * @param fileName The name of the file to create.
 * @return RC_OK on success, RC_FILE_NOT_FOUND if the file cannot be created.
 */
RC createPageFile(char *fileName) {
    // Open the file in writing mode ("w"). This will create the file if it doesn't exist, or overwrite it if it does.
    FILE *file = fopen(fileName, "w");
    if (!file) return RC_FILE_NOT_FOUND; // Return error if the file cannot be opened
    // Allocate memory for an empty page
    char *emptyPage = calloc(PAGE_SIZE, sizeof(char)); // calloc initializes the memory to 0
    // Write the empty page to the file
    fwrite(emptyPage, sizeof(char), PAGE_SIZE, file);
    // Free the allocated memory
    free(emptyPage);
    // Close the file
    fclose(file);
    return RC_OK; // Return success
}

/*
 * Opens an existing page file.
 *
 * @param fileName The name of the file to open.
 * @param fHandle A pointer to the SM_FileHandle structure to be populated.
 * @return RC_OK on success, RC_FILE_NOT_FOUND if the file cannot be opened.
 */
RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    // Open the file in read and update mode ("r+").  This allows both reading and writing.
    FILE *file = fopen(fileName, "r+");
    if (!file) return RC_FILE_NOT_FOUND; // Return error if the file cannot be opened

    // Get the file size and calculate the total number of pages
    fseek(file, 0, SEEK_END); // Move a file pointer to the end
    int totalNumPages = ftell(file) / PAGE_SIZE; // Calculate the total number of pages

    // Populate the file handle structure
    fHandle->fileName = fileName; // Store the file name
    fHandle->totalNumPages = totalNumPages; // Store the total number of pages
    fHandle->curPagePos = 0; // Initialize current page position to 0
    fHandle->mgmtInfo = file; // Store the file pointer in the management info

    return RC_OK; // Return success
}

/*
 * Closes an open page file.
 *
 * @param fHandle A pointer to the SM_FileHandle structure.
 * @return RC_OK on success.
 */
RC closePageFile(SM_FileHandle *fHandle) {
    // Get the file pointer from the file handle
    FILE *file = (FILE *) fHandle->mgmtInfo;
    // Close the file
    fclose(file);
    return RC_OK; // Return success
}

/*
 * Destroys a page file.
 *
 * @param fileName The name of the file to destroy.
 * @return RC_OK on success, RC_FILE_NOT_FOUND if the file cannot be removed.
 */
RC destroyPageFile(char *fileName) {
    // Remove the file using the remove() function
    return (remove(fileName) == 0) ? RC_OK : RC_FILE_NOT_FOUND; // Return appropriate error code
}

/*
 * Reads a specific page from the file.
 *
 * @param pageNum The page number to read.
 * @param fHandle A pointer to the SM_FileHandle structure.
 * @param memPage A pointer to the memory page where the data will be stored.
 * @return RC_OK on success, RC_READ_NON_EXISTING_PAGE if the page number is invalid.
 */
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Get the file pointer from the file handle
    FILE *file = (FILE *) fHandle->mgmtInfo;
    // Check for invalid file pointer or page number
    if (!file || pageNum < 0 || pageNum >= fHandle->totalNumPages)
        return RC_READ_NON_EXISTING_PAGE;

    // Seek to the correct position in the file
    fseek(file, pageNum * PAGE_SIZE, SEEK_SET); // SEEK_SET: from the beginning of the file
    // Read the page into memory
    fread(memPage, sizeof(char), PAGE_SIZE, file);
    // Update the current page position in the file handle
    fHandle->curPagePos = pageNum;

    return RC_OK; // Return success
}

/*
 * Gets the current position in the file.
 *
 * @param fHandle A pointer to the SM_FileHandle structure.
 * @return The current page position.
 */
int getBlockPos(SM_FileHandle *fHandle) {
    return fHandle->curPagePos;
}

/*
 * Reads the first block of the file.
 *
 * @param fHandle A pointer to the SM_FileHandle structure.
 * @param memPage A pointer to the memory page where the data will be stored.
 * @return RC_OK on success, RC_READ_NON_EXISTING_PAGE if the file is empty.
 */

/*
 * Reads the first block of the file.
 *
 * @param fHandle A pointer to the SM_FileHandle structure.
 * @param memPage A pointer to the memory page where the data will be stored.
 * @return RC_OK on success, RC_READ_NON_EXISTING_PAGE if the file is empty.
 */
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(0, fHandle, memPage); // Call readBlock with page number 0
}

/*
 * Reads the previous block of the file.
 *
 * @param fHandle A pointer to the SM_FileHandle structure.
 * @param memPage A pointer to the memory page where the data will be stored.
 * @return RC_OK on success, RC_READ_NON_EXISTING_PAGE if already at the first block.
 */
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle->curPagePos == 0) return RC_READ_NON_EXISTING_PAGE; // Check if already at the first page
    return readBlock(fHandle->curPagePos - 1, fHandle, memPage); // Call readBlock with the previous page number
}

/*
 * Reads the current block of the file.
 *
 * @param fHandle A pointer to the SM_FileHandle structure.
 * @param memPage A pointer to the memory page where the data will be stored.
 * @return RC_OK on success.
 */
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->curPagePos, fHandle, memPage); // Call readBlock with the current page number
}

/*
 * Reads the next block of the file.
 *
 * @param fHandle A pointer to the SM_FileHandle structure.
 * @param memPage A pointer to the memory page where the data will be stored.
 * @return RC_OK on success, RC_READ_NON_EXISTING_PAGE if already at the last block.
 */
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle->curPagePos + 1 >= fHandle->totalNumPages) return RC_READ_NON_EXISTING_PAGE;
    // Check if already at the last page
    return readBlock(fHandle->curPagePos + 1, fHandle, memPage); // Call readBlock with the next page number
}

/*
 * Reads the last block of the file.
 *
 * @param fHandle A pointer to the SM_FileHandle structure.
 * @param memPage A pointer to the memory page where the data will be stored.
 * @return RC_OK on success.
 */
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->totalNumPages - 1, fHandle, memPage); // Call readBlock with the last page number
}

/*
 * Writes a specific page to the file.
 *
 * @param pageNum The page number to write.
 * @param fHandle A pointer to the SM_FileHandle structure.
 * @param memPage A pointer to the memory page containing the data to write.
 * @return RC_OK on success, RC_WRITE_FAILED if the page number is invalid.
 */
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    FILE *file = (FILE *) fHandle->mgmtInfo;
    if (!file || pageNum < 0 || pageNum >= fHandle->totalNumPages)
        return RC_WRITE_FAILED;

    fseek(file, pageNum * PAGE_SIZE, SEEK_SET);
    fwrite(memPage, sizeof(char), PAGE_SIZE, file);
    fHandle->curPagePos = pageNum;

    return RC_OK;
}

/*
 * Writes the current block (page) in the file.
 *
 * @param fHandle A pointer to the SM_FileHandle structure.
 * @param memPage A pointer to the memory page containing the data to write.
 * @return RC_OK on success, or an appropriate error code if writing fails (e.g., if the current position is invalid).
 */
RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // Call the writeBlock function to perform the actual write operation.
    // The current page position is obtained from the file handle's curPagePos member.
    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

/*
 * Appends an empty block (page) to the end of the file.
 *
 * @param fHandle A pointer to the SM_FileHandle structure representing the file.
 * @return RC_OK on success, RC_FILE_HANDLE_NOT_INIT if the file handle is invalid.
 */
RC appendEmptyBlock(SM_FileHandle *fHandle) {
    // Get the file pointer from the file handle's management info
    FILE *file = (FILE *) fHandle->mgmtInfo;

    // Check if the file pointer is valid
    if (!file) return RC_FILE_HANDLE_NOT_INIT;

    // Allocate memory for an empty page using calloc (initializes to 0)
    char *emptyPage = calloc(PAGE_SIZE, sizeof(char));

    // Seek to the end of the file
    fseek(file, 0, SEEK_END);

    // Write the empty page to the file
    fwrite(emptyPage, sizeof(char), PAGE_SIZE, file);

    // Free the allocated memory for the empty page
    free(emptyPage);

    // Increment the total number of pages in the file handle
    fHandle->totalNumPages++;

    return RC_OK; // Return success
}

/*
 * Ensures that the file has at least the specified number of pages.
 * If the file has fewer pages, it appends empty pages until the desired capacity is reached.
 *
 * @param numberOfPages The minimum number of pages the file should have.
 * @param fHandle A pointer to the SM_FileHandle structure.
 * @return RC_OK on success, or an error code if appending fails.
 */
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    // Loop until the file has the desired number of pages
    while (fHandle->totalNumPages < numberOfPages) {
        // Append an empty block to the file
        RC rc = appendEmptyBlock(fHandle);

        // If appending fails, return the error code
        if (rc != RC_OK) return rc;
    }

    return RC_OK; // Return success
}
