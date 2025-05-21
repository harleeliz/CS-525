/************************************************************
 *     File name:                storage_mgr.c
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "storage_mgr.h"
#include "dberror.h"

/*
 * Initializes the Storage Manager.
 * Called once at the start of program execution.
 */
void initStorageManager(void) {
    printf("Storage Manager initialized.\n");
}

/*
 * Creates a new page file with one empty page.
 */
RC createPageFile(char *fileName) {
    if (fileName == NULL)
        return RC_FILE_NOT_FOUND;

    FILE *fp = fopen(fileName, "w+");
    if (fp == NULL)
        return RC_FILE_NOT_FOUND;

    char *emptyPage = (char *) calloc(PAGE_SIZE, sizeof(char));
    if (!emptyPage) {
        fclose(fp);
        return RC_MALLOC_FAILED;
    }

    fwrite(emptyPage, sizeof(char), PAGE_SIZE, fp);
    fclose(fp);
    free(emptyPage);
    return RC_OK;
}

/*
 * Opens an existing page file and populates the file handle.
 */
RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    if (fileName == NULL || fHandle == NULL)
        return RC_FILE_NOT_FOUND;

    FILE *fp = fopen(fileName, "r+");
    if (fp == NULL)
        return RC_FILE_NOT_FOUND;

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return RC_READ_NON_EXISTING_PAGE;
    }

    fHandle->mgmtInfo = fp;
    fHandle->fileName = fileName;
    fHandle->curPagePos = 0;
    long fileSize = ftell(fp);
    fHandle->totalNumPages = (int)(fileSize / PAGE_SIZE);
    return RC_OK;
}

/*
 * Closes an open page file.
 */
RC closePageFile(SM_FileHandle *fHandle) {
    if (fHandle == NULL)
        return RC_FILE_HANDLE_NOT_INIT;

    FILE *fp = fHandle->mgmtInfo;
    fclose(fp);
    return RC_OK;
}

/*
 * Destroys (deletes) a page file.
 */
RC destroyPageFile(char *fileName) {
    if (fileName == NULL)
        return RC_FILE_NOT_FOUND;
    if (remove(fileName) != 0)
        return RC_FILE_NOT_FOUND;
    return RC_OK;
}

/*
 * Reads the specified page from the file into memPage.
 */
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL || memPage == NULL)
        return RC_FILE_HANDLE_NOT_INIT;

    if (pageNum < 0 || pageNum >= fHandle->totalNumPages)
        return RC_READ_NON_EXISTING_PAGE;

    FILE *fp = fHandle->mgmtInfo;
    if (fp == NULL)
        return RC_FILE_NOT_FOUND;

    int offset = pageNum * PAGE_SIZE;
    if (fseek(fp, offset, SEEK_SET) != 0)
        return RC_READ_NON_EXISTING_PAGE;

    fread(memPage, sizeof(char), PAGE_SIZE, fp);
    fHandle->curPagePos = pageNum;
    return RC_OK;
}

/*
 * Returns the current page position.
 */
int getBlockPos(SM_FileHandle *fHandle) {
    if (fHandle == NULL)
        return -1;
    return fHandle->curPagePos;
}

/*
 * Reads the first block of the file.
 */
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    return readBlock(0, fHandle, memPage);
}

/*
 * Reads the previous block relative to the current page position.
 */
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int pageNum = getBlockPos(fHandle);
    if (pageNum <= 0)
        return RC_READ_NON_EXISTING_PAGE;
    return readBlock(pageNum - 1, fHandle, memPage);
}

/*
 * Reads the current block.
 */
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int pageNum = getBlockPos(fHandle);
    if (pageNum < 0)
        return RC_FILE_HANDLE_NOT_INIT;
    return readBlock(pageNum, fHandle, memPage);
}

/*
 * Reads the next block relative to the current page position.
 */
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int pageNum = getBlockPos(fHandle);
    if (pageNum < 0 || pageNum + 1 >= fHandle->totalNumPages)
        return RC_READ_NON_EXISTING_PAGE;
    return readBlock(pageNum + 1, fHandle, memPage);
}

/*
 * Reads the last block of the file.
 */
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    int lastPage = fHandle->totalNumPages - 1;
    return readBlock(lastPage, fHandle, memPage);
}

/*
 * Writes a specific page (block) from memPage to the file.
 */
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL || memPage == NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages)
        return RC_READ_NON_EXISTING_PAGE;

    FILE *fp = fHandle->mgmtInfo;
    if (fp == NULL)
        return RC_FILE_NOT_FOUND;

    int offset = pageNum * PAGE_SIZE;
    if (fseek(fp, offset, SEEK_SET) != 0)
        return RC_READ_NON_EXISTING_PAGE;

    /* Write the page content.
       Note: Here we write PAGE_SIZE bytes; adjust if needed. */
    fwrite(memPage, sizeof(char), PAGE_SIZE, fp);
    fHandle->curPagePos = pageNum;
    return RC_OK;
}

/*
 * Writes the current block.
 */
RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    int curPage = fHandle->curPagePos;
    return writeBlock(curPage, fHandle, memPage);
}

/*
 * Appends an empty block (page) to the file.
 */
RC appendEmptyBlock(SM_FileHandle *fHandle) {
    FILE *fp = (FILE *) fHandle->mgmtInfo;
    if (!fp)
        return RC_FILE_HANDLE_NOT_INIT;
    char *emptyPage = calloc(PAGE_SIZE, sizeof(char));
    if (!emptyPage)
        return RC_MALLOC_FAILED;
    fseek(fp, 0, SEEK_END);
    fwrite(emptyPage, sizeof(char), PAGE_SIZE, fp);
    free(emptyPage);
    fHandle->totalNumPages++;
    return RC_OK;
}

/*
 * Ensures that the file has at least the specified number of pages.
 */
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
    if (fHandle == NULL)
        return RC_FILE_HANDLE_NOT_INIT;
    if (numberOfPages < 1)
        return RC_READ_NON_EXISTING_PAGE;

    int currentPages = fHandle->totalNumPages;
    for (int i = 0; i < numberOfPages - currentPages; i++) {
        appendEmptyBlock(fHandle);
    }
    if (fHandle->totalNumPages < numberOfPages)
        return RC_WRITE_FAILED;
    return RC_OK;
}