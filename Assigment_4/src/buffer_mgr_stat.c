/************************************************************
 * File name:      buffer_mgr_stat.c
 * CS 525 Advanced Database Organization (Spring 2025)
 * Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This source file implements the debugging and statistics
 *   interface for the Buffer Manager. It includes functions to
 *   print the contents of the buffer pool and individual pages,
 *   as well as functions to retrieve statistics such as frame
 *   contents, dirty flags, fix counts, and I/O counts.
 ************************************************************/

#include "buffer_mgr_stat.h"
#include "buffer_mgr.h"
#include "dberror.h"
#include "storage_mgr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Local helper function to print the replacement strategy */
static void printStrat(BM_BufferPool *const bm) {
    if (bm == NULL) {
        printf("No strategy (bm is NULL)");
        return;
    }
    switch (bm->strategy) {
        case RS_FIFO:
            printf("FIFO");
            break;
        case RS_LRU:
            printf("LRU");
            break;
        case RS_CLOCK:
            printf("CLOCK");
            break;
        case RS_LFU:
            printf("LFU");
            break;
        case RS_LRU_K:
            printf("LRU-K");
            break;
        default:
            printf("%i", bm->strategy);
            break;
    }
}

/*
 * printPoolContent:
 *   Prints a formatted representation of the buffer pool's state,
 *   including the replacement strategy, page numbers stored in each
 *   frame, dirty flags, and fix counts.
 */
void printPoolContent(BM_BufferPool *const bm) {
    if (bm == NULL || bm->mgmtData == NULL) {
        printf("Buffer pool is not initialized.\n");
        return;
    }


    int numPages = bm->numPages;
    PageNumber *frameContent = getFrameContents(bm);
    int *fixCounts = getFixCounts(bm);
    bool *dirtyFlags = getDirtyFlags(bm);

    if (!frameContent || !fixCounts || !dirtyFlags) {
        printf("Failed to retrieve statistics.\n");
        free(frameContent);
        free(fixCounts);
        free(dirtyFlags);
        return;
    }

    printf("{");
    printStrat(bm);
    printf(" %i}: ", numPages);
    for (int i = 0; i < numPages; i++) {
        printf("%s[%i%s%i]", (i == 0 ? "" : ","), frameContent[i],
               (dirtyFlags[i] ? "x" : " "), fixCounts[i]);
    }
    printf("\n");

    free(frameContent);
    free(fixCounts);
    free(dirtyFlags);
}

/*
 * sprintPoolContent:
 *   Returns a string representation of the buffer pool's state.
 *   The caller is responsible for freeing the allocated memory.
 */
char *sprintPoolContent(BM_BufferPool *const bm) {
    if (bm == NULL || bm->mgmtData == NULL)
        return NULL;

    int numPages = bm->numPages;
    PageNumber *frameContent = getFrameContents(bm);
    int *fixCounts = getFixCounts(bm);
    bool *dirtyFlags = getDirtyFlags(bm);

    if (!frameContent || !fixCounts || !dirtyFlags) {
        free(frameContent);
        free(fixCounts);
        free(dirtyFlags);
        return NULL;
    }

    size_t bufSize = 256 + (22 * numPages);
    char *message = (char *) malloc(bufSize);
    if (message == NULL) {
        free(frameContent);
        free(fixCounts);
        free(dirtyFlags);
        return NULL;
    }
    int pos = 0;
    for (int i = 0; i < numPages; i++) {
        pos += snprintf(message + pos, bufSize - pos, "%s[%i%s%i]",
                        (i == 0 ? "" : ","), frameContent[i],
                        (dirtyFlags[i] ? "x" : " "), fixCounts[i]);
    }

    free(frameContent);
    free(fixCounts);
    free(dirtyFlags);
    return message;
}

/*
 * printPageContent:
 *   Prints a formatted hexadecimal representation of the content of a page.
 */
void printPageContent(BM_PageHandle *const page) {
    if (page == NULL) {
        printf("Page is NULL.\n");
        return;
    }
    printf("[Page %i]\n", page->pageNum);
    for (int i = 0; i < PAGE_SIZE; i++) {
        printf("%02X%s", (unsigned char) page->data[i],
               (((i + 1) % 8 == 0) ? " " : ""));
        if ((i + 1) % 64 == 0)
            printf("\n");
    }
    printf("\n");
}

/*
 * sprintPageContent:
 *   Returns a string with a formatted hexadecimal representation of the page's content.
 *   The caller is responsible for freeing the returned string.
 */
char *sprintPageContent(BM_PageHandle *const page) {
    if (page == NULL)
        return NULL;
    size_t bufSize = 30 + (2 * PAGE_SIZE) + (PAGE_SIZE / 8) + (PAGE_SIZE / 64);
    char *message = (char *) malloc(bufSize);
    if (message == NULL)
        return NULL;
    int pos = 0;
    pos += snprintf(message + pos, bufSize - pos, "[Page %i]\n", page->pageNum);
    for (int i = 0; i < PAGE_SIZE; i++) {
        pos += snprintf(message + pos, bufSize - pos, "%02X%s",
                        (unsigned char) page->data[i],
                        (((i + 1) % 8 == 0) ? " " : ""));
        if ((i + 1) % 64 == 0)
            pos += snprintf(message + pos, bufSize - pos, "\n");
    }
    return message;
}

/*
 * getFrameContents:
 *   Returns an array of PageNumbers (one for each frame) indicating the page stored in each frame.
 *   For empty frames, the value is NO_PAGE.
 *   The caller must free the returned array.
 */
PageNumber *getFrameContents(BM_BufferPool *const bm) {
    if (bm == NULL || bm->mgmtData == NULL)
        return NULL;

    BM_MgmtData *mgmt = (BM_MgmtData *) bm->mgmtData;
    int numPages = bm->numPages;
    PageNumber *arr = (PageNumber *) malloc(numPages * sizeof(PageNumber));
    if (arr == NULL)
        return NULL;

    for (int i = 0; i < numPages; i++) {
        arr[i] = mgmt->frames[i].pageNum;
    }
    return arr;
}

/*
 * getDirtyFlags:
 *   Returns an array of booleans (one for each frame) indicating whether the frame is dirty.
 *   The caller must free the allocated memory.
 */
bool *getDirtyFlags(BM_BufferPool *const bm) {
    if (bm == NULL || bm->mgmtData == NULL)
        return NULL;

    BM_MgmtData *mgmt = (BM_MgmtData *) bm->mgmtData;
    int numPages = bm->numPages;
    bool *arr = (bool *) malloc(numPages * sizeof(bool));
    if (arr == NULL)
        return NULL;

    for (int i = 0; i < numPages; i++) {
        arr[i] = mgmt->frames[i].dirty;
    }
    return arr;
}

/*
 * getFixCounts:
 *   Returns an array of integers (one for each frame) indicating the fixCount (pin count) for each frame.
 *   The caller must free the allocated memory.
 */
int *getFixCounts(BM_BufferPool *const bm) {
    if (bm == NULL || bm->mgmtData == NULL)
        return NULL;

    BM_MgmtData *mgmt = (BM_MgmtData *) bm->mgmtData;
    int numPages = bm->numPages;
    int *arr = (int *) malloc(numPages * sizeof(int));
    if (arr == NULL)
        return NULL;

    for (int i = 0; i < numPages; i++) {
        arr[i] = mgmt->frames[i].fixCount;
    }
    return arr;
}

/*
 * getNumReadIO:
 *   Returns the total number of read I/O operations performed on the buffer pool.
 */
int getNumReadIO(BM_BufferPool *const bm) {
    if (bm == NULL || bm->mgmtData == NULL)
        return -1;
    BM_MgmtData *mgmt = (BM_MgmtData *) bm->mgmtData;
    return mgmt->readIO;
}

/*
 * getNumWriteIO:
 *   Returns the total number of write I/O operations performed on the buffer pool.
 */
int getNumWriteIO(BM_BufferPool *const bm) {
    if (bm == NULL || bm->mgmtData == NULL)
        return -1;
    BM_MgmtData *mgmt = (BM_MgmtData *) bm->mgmtData;
    return mgmt->writeIO;
}