/************************************************************
*     File name:               storage_mgr.c                *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/


#include "storage_mgr.h"
#include "dberror.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize the storage manager
void initStorageManager(void) {
    // Initialization logic if needed
}

// Create a new page file with one zero-initialized page
RC createPageFile(char *fileName) {
    FILE *fp = fopen(fileName, "w+");
    if (fp == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    char *emptyPage = (char *) calloc(PAGE_SIZE, sizeof(char));
    if (emptyPage == NULL) {
        fclose(fp);
        return RC_WRITE_FAILED;
    }

    size_t bytesWritten = fwrite(emptyPage, sizeof(char), PAGE_SIZE, fp);
    free(emptyPage);
    fclose(fp);

    if (bytesWritten != PAGE_SIZE) {
        return RC_WRITE_FAILED;
    }

    return RC_OK;
}

// Open an existing page file and initialize the file handle
RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    FILE *fp = fopen(fileName, "r+");
    if (fp == NULL) {
        return RC_FILE_NOT_FOUND;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    if (fileSize % PAGE_SIZE != 0) {
        fclose(fp);
        return RC_READ_NON_EXISTING_PAGE;
    }

    fHandle->fileName = fileName;
    fHandle->totalNumPages = fileSize / PAGE_SIZE;
    fHandle->curPagePos = 0;
    fHandle->mgmtInfo = fp;

    return RC_OK;
}

// Close an open page file
RC closePageFile(SM_FileHandle *fHandle) {
    if (fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    fclose((FILE *) fHandle->mgmtInfo);
    fHandle->mgmtInfo = NULL;
    return RC_OK;
}

// Destroy a page file
RC destroyPageFile(char *fileName) {
    if (remove(fileName) != 0) {
        return RC_FILE_NOT_FOUND;
    }
    return RC_OK;
}

// Read a block from the specified page number
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        return RC_READ_NON_EXISTING_PAGE;
    }

    FILE *fp = (FILE *) fHandle->mgmtInfo;
    if (fseek(fp, pageNum * PAGE_SIZE, SEEK_SET) != 0) {
        return RC_READ_NON_EXISTING_PAGE;
    }

    size_t bytesRead = fread(memPage, sizeof(char), PAGE_SIZE, fp);
    if (bytesRead != PAGE_SIZE) {
        return RC_READ_NON_EXISTING_PAGE;
    }

    fHandle->curPagePos = pageNum;
    return RC_OK;
}

// Get the current page position
int getBlockPos(SM_FileHandle *fHandle) {
    return fHandle->curPagePos;
}

// Read the first block
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(0, fHandle, memPage);
}

// Read the previous block relative to current position
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle->curPagePos <= 0) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    return readBlock(fHandle->curPagePos - 1, fHandle, memPage);
}

// Read the current block
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->curPagePos, fHandle, memPage);
}

// Read the next block relative to current position
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle->curPagePos >= fHandle->totalNumPages - 1) {
        return RC_READ_NON_EXISTING_PAGE;
    }
    return readBlock(fHandle->curPagePos + 1, fHandle, memPage);
}

// Read the last block
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}

// Write a block to the specified page number
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        return RC_WRITE_FAILED;
    }

    FILE *fp = (FILE *) fHandle->mgmtInfo;
    if (fseek(fp, pageNum * PAGE_SIZE, SEEK_SET) != 0) {
        return RC_WRITE_FAILED;
    }

    size_t bytesWritten = fwrite(memPage, sizeof(char), PAGE_SIZE, fp);
    if (bytesWritten != PAGE_SIZE) {
        return RC_WRITE_FAILED;
    }

    fHandle->curPagePos = pageNum;
    return RC_OK;
}

// Write to the current block
RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return writeBlock(fHandle->curPagePos, fHandle, memPage);
}

// Append an empty block to the file
RC appendEmptyBlock(SM_FileHandle *fHandle) {
    if (fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;
    }

    FILE *fp = (FILE *) fHandle->mgmtInfo;
    char *emptyPage = (char *) calloc(PAGE_SIZE, sizeof(char));
    if (emptyPage == NULL) {
        return RC_WRITE_FAILED;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        free(emptyPage);
        return RC_WRITE_FAILED;
    }

    size_t bytesWritten = fwrite(emptyPage, sizeof(char), PAGE_SIZE, fp);
    free(emptyPage);

    if (bytesWritten != PAGE_SIZE) {
        return RC_WRITE_FAILED;
    }

    fHandle->totalNumPages++;
    return RC_OK;
}

// Ensure the file has at least the specified number of pages
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    if (numberOfPages < 0) {
        return RC_OK; // Invalid input, treat as success or return error
    }

    while (fHandle->totalNumPages < numberOfPages) {
        RC rc = appendEmptyBlock(fHandle);
        if (rc != RC_OK) {
            return rc;
        }
    }
    return RC_OK;
}
