/************************************************************
 *     File name:                storage_mgr.h
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/
#ifndef STORAGE_MGR_H
#define STORAGE_MGR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dberror.h"

/* --- Handle Data Structures --- */

/* File handle for a page file */
typedef struct SM_FileHandle {
    char *fileName;       /* Name of the file */
    int totalNumPages;    /* Total number of pages in the file */
    int curPagePos;       /* Current page position */
    void *mgmtInfo;       /* Management information (implementation-specific) */
} SM_FileHandle;

/* Page handle: pointer to a page's data in memory */
typedef char *SM_PageHandle;

/* --- Interface Functions --- */

/* Storage Manager Initialization */
extern void initStorageManager(void);

/* Page File Operations */
extern RC createPageFile(char *fileName);
extern RC openPageFile(char *fileName, SM_FileHandle *fHandle);
extern RC closePageFile(SM_FileHandle *fHandle);
extern RC destroyPageFile(char *fileName);

/* Reading Blocks from Disk */
extern RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);
extern int getBlockPos(SM_FileHandle *fHandle);
extern RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);

/* Writing Blocks to Disk */
extern RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage);
extern RC appendEmptyBlock(SM_FileHandle *fHandle);
extern RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle);

#ifdef __cplusplus
}
#endif

#endif // STORAGE_MGR_H