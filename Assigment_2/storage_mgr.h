/************************************************************
*     File name:                storage_mgr.h               *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/

#ifndef STORAGE_MGR_H  // Include guard to prevent multiple inclusions of this header file
#define STORAGE_MGR_H

#include "dberror.h" // Include the dberror.h header file for error handling

/************************************************************
 *                    Handle data structures                *
 ************************************************************/

// Structure representing a file handle.  Provides an interface for interacting with a page file.
typedef struct SM_FileHandle {
    char *fileName; // Name of the file
    int totalNumPages; // Total number of pages in the file
    int curPagePos; // Current position of the file pointer (in pages)
    void *mgmtInfo; // Pointer to management information (implementation-specific data)
} SM_FileHandle;

// Type definition for a page handle.  A page handle is essentially a pointer to a memory buffer (a page).
typedef char *SM_PageHandle; //  A page handle is a pointer to the data buffer of a page.

/************************************************************
 *                    Interface                             *
 ************************************************************/

/* Manipulating page files (Functions for creating, opening, closing, and destroying page files) */

// Initializes the storage manager.  This function is called once at the start of the program's execution.
extern void initStorageManager(void);

// Creates a new page file with a single empty page.
extern RC createPageFile(char *fileName);

// Opens an existing page file.  Populates the file handle with information about the file.
extern RC openPageFile(char *fileName, SM_FileHandle *fHandle);

// Closes an open page file.  Releases resources associated with the file.
extern RC closePageFile(SM_FileHandle *fHandle);

// Destroys a page file.  Deletes the file from the file system.
extern RC destroyPageFile(char *fileName);

/* Reading blocks from disk (Functions for reading pages from the file) */

// Reads a specific block (page) from the file into memory.
extern RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);

// Gets the current position of the file pointer (in pages).
extern int getBlockPos(SM_FileHandle *fHandle);

// Reads the first block (page) of the file into memory.
extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

// Reads the previous block (page) relative to the current position.
extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

// Reads the current block (page) into memory.
extern RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

// Reads the next block (page) relative to the current position.
extern RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

// Reads the last block (page) of the file into memory.
extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/* Writing blocks to a page file (Functions for writing pages to the file) */

// Writes a specific block (page) from memory to the file.
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);

// Writes the current block (page) from memory to the file.
extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

// Appends an empty block (page) to the end of the file.
extern RC appendEmptyBlock(SM_FileHandle *fHandle);

// Ensures that the file has at least the specified number of pages.  If the file has fewer pages, it appends empty pages until the desired capacity is reached.
extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle);

#endif // End of include guard
