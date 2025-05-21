/************************************************************
 * File name:      storage_mgr.h
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This header file defines the interface for the Storage Manager,
 *   a low-level module responsible for managing page files on disk.
 *   It includes data structures for file handles and page handles,
 *   as well as function prototypes for creating, opening, closing,
 *   destroying, reading, and writing page files.
 ************************************************************/

#ifndef STORAGE_MGR_H
#define STORAGE_MGR_H

#include "dberror.h"  // Provides error codes and error handling mechanisms

/************************************************************
 *                   Handle Data Structures                 *
 ************************************************************/

/*
 * Structure: SM_FileHandle
 * ------------------------
 * Represents an open page file.
 *
 * Members:
 *   fileName      - The name of the file.
 *   totalNumPages - The total number of pages in the file.
 *   curPagePos    - The current page position in the file.
 *   mgmtInfo      - Pointer to additional management information
 *                   (implementation specific).
 */
typedef struct SM_FileHandle {
    char *fileName;
    int totalNumPages;
    int curPagePos;
    void *mgmtInfo;
} SM_FileHandle;

/*
 * Type: SM_PageHandle
 * -------------------
 * A pointer to a character array representing a page in memory.
 */
typedef char* SM_PageHandle;

/************************************************************
 *                      Interface Functions                 *
 ************************************************************/

/*
 * Function: initStorageManager
 * ----------------------------
 * Initializes the storage manager.
 * This function should be called once before any other storage
 * manager operations are performed.
 */
extern void initStorageManager(void);

/*
 * Function: createPageFile
 * ------------------------
 * Creates a new page file with one page (of size PAGE_SIZE).
 *
 * Parameters:
 *   fileName - Name of the file to be created.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC createPageFile(char *fileName);

/*
 * Function: openPageFile
 * ----------------------
 * Opens an existing page file and populates the SM_FileHandle structure.
 *
 * Parameters:
 *   fileName - Name of the file to open.
 *   fHandle  - Pointer to an SM_FileHandle structure that will be
 *              populated with file information.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC openPageFile(char *fileName, SM_FileHandle *fHandle);

/*
 * Function: closePageFile
 * -----------------------
 * Closes an open page file.
 *
 * Parameters:
 *   fHandle - Pointer to the SM_FileHandle of the file to be closed.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC closePageFile(SM_FileHandle *fHandle);

/*
 * Function: destroyPageFile
 * -------------------------
 * Deletes a page file from disk.
 *
 * Parameters:
 *   fileName - Name of the file to be deleted.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC destroyPageFile(char *fileName);

/*
 * Function: readBlock
 * -------------------
 * Reads a block (page) from the file at the specified page number
 * into memory.
 *
 * Parameters:
 *   pageNum  - The page number to read (0-indexed).
 *   fHandle  - Pointer to the SM_FileHandle representing the file.
 *   memPage  - Pointer to a buffer where the page data will be stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);

/*
 * Function: getBlockPos
 * ---------------------
 * Returns the current page position in the file.
 *
 * Parameters:
 *   fHandle - Pointer to the SM_FileHandle.
 *
 * Returns:
 *   The current page position.
 */
extern int getBlockPos(SM_FileHandle *fHandle);

/*
 * Function: readFirstBlock
 * ------------------------
 * Reads the first block (page) of the file into memory.
 *
 * Parameters:
 *   fHandle - Pointer to the SM_FileHandle.
 *   memPage - Pointer to a buffer where the page data will be stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/*
 * Function: readPreviousBlock
 * ---------------------------
 * Reads the block preceding the current page position.
 *
 * Parameters:
 *   fHandle - Pointer to the SM_FileHandle.
 *   memPage - Pointer to a buffer where the page data will be stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/*
 * Function: readCurrentBlock
 * --------------------------
 * Reads the current block (page) into memory.
 *
 * Parameters:
 *   fHandle - Pointer to the SM_FileHandle.
 *   memPage - Pointer to a buffer where the page data will be stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/*
 * Function: readNextBlock
 * -----------------------
 * Reads the block following the current page position.
 *
 * Parameters:
 *   fHandle - Pointer to the SM_FileHandle.
 *   memPage - Pointer to a buffer where the page data will be stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/*
 * Function: readLastBlock
 * -----------------------
 * Reads the last block (page) of the file into memory.
 *
 * Parameters:
 *   fHandle - Pointer to the SM_FileHandle.
 *   memPage - Pointer to a buffer where the page data will be stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/*
 * Function: writeBlock
 * --------------------
 * Writes a block (page) to the file at the specified page number.
 *
 * Parameters:
 *   pageNum  - The page number to write to (0-indexed).
 *   fHandle  - Pointer to the SM_FileHandle.
 *   memPage  - Pointer to the buffer containing the page data to write.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);

/*
 * Function: writeCurrentBlock
 * ---------------------------
 * Writes the current block (page) to the file.
 *
 * Parameters:
 *   fHandle - Pointer to the SM_FileHandle.
 *   memPage - Pointer to the buffer containing the page data.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/*
 * Function: appendEmptyBlock
 * --------------------------
 * Appends an empty block (page) to the end of the file.
 *
 * Parameters:
 *   fHandle - Pointer to the SM_FileHandle.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC appendEmptyBlock(SM_FileHandle *fHandle);

/*
 * Function: ensureCapacity
 * ------------------------
 * Ensures that the file has at least the specified number of pages.
 * If the file has fewer pages, empty blocks are appended until the
 * required capacity is reached.
 *
 * Parameters:
 *   numberOfPages - The minimum number of pages the file should have.
 *   fHandle       - Pointer to the SM_FileHandle.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle);

#endif