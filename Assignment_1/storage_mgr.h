/************************************************************
*     File name:            storage_mgr.h                   *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/

#ifndef STORAGE_MGR_H
#define STORAGE_MGR_H
#include "dberror.h"

/************************************************************
 *                    Handle Data Structures                *
 ************************************************************/

/* SM_FileHandle: Represents an open file. It stores metadata like file name,
   total number of pages, current page position, and additional bookkeeping info. */
typedef struct SM_FileHandle {
    char *fileName; // Name of the file
    int totalNumPages; // Total number of pages in the file
    int curPagePos; // Current position within the file (in pages)
    void *mgmtInfo; // Pointer to additional file management info
} SM_FileHandle;

/* SM_PageHandle: Pointer to a block of memory that stores the data of a page. */
typedef char *SM_PageHandle;

/************************************************************
 *                    Interface                             *
 ************************************************************/

/* Initialize the storage manager. This function should be called before
   any other storage manager functions to set up the environment. */
extern void initStorageManager(void);

/* Create a new page file with the specified name. The file should be initialized
   with a single page filled with `\0` bytes. */
extern RC createPageFile(char *fileName);

/* Open an existing page file. The metadata of the file should be stored in the
   provided SM_FileHandle structure. */
extern RC openPageFile(char *fileName, SM_FileHandle *fHandle);

/* Close the given page file and clean up associated resources. */
extern RC closePageFile(SM_FileHandle *fHandle);

/* Destroy (delete) the specified page file from the system. */
extern RC destroyPageFile(char *fileName);

/************************************************************
 *                Reading Blocks from Disk                  *
 ************************************************************/

/* Read the block at position `pageNum` from the file and store its content
   into the memory pointed to by `memPage`. */
extern RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);

/* Get the current page position in the file. Returns the page number. */
extern int getBlockPos(SM_FileHandle *fHandle);

/* Read the first block of the file into the memory pointed to by `memPage`. */
extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/* Read the block immediately before the current position. */
extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/* Read the block at the current position in the file. */
extern RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/* Read the block immediately after the current position. */
extern RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/* Read the last block of the file into the memory pointed to by `memPage`. */
extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/************************************************************
 *                Writing Blocks to a Page File             *
 ************************************************************/

/* Write the data pointed to by `memPage` to the block at position `pageNum`. */
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);

/* Write the data pointed to by `memPage` to the current block in the file. */
extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/* Append an empty block (filled with `\0` bytes) to the end of the file. */
extern RC appendEmptyBlock(SM_FileHandle *fHandle);

/* Ensure the file has at least `numberOfPages` pages. If not, expand it
   by appending empty blocks until the required size is met. */
extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle);

#endif // STORAGE_MGR_H
