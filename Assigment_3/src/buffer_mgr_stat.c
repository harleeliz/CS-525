/************************************************************
 * File name:      buffer_mgr_stat.c
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
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
#include <stdio.h>
#include <stdlib.h>

// Local helper function to print the replacement strategy used
static void printStrat(BM_BufferPool *const bm);

/*
 * Function: printPoolContent
 * --------------------------
 * Prints a formatted representation of the buffer pool's state,
 * including the replacement strategy, page numbers stored in each
 * frame, dirty flags, and fix counts.
 */
void printPoolContent(BM_BufferPool *const bm)
{
    PageNumber *frameContent;
    int *dirty;
    int *fixCount;
    int i;

    // Retrieve statistics from the buffer manager
    frameContent = getFrameContents(bm);
    dirty = getDirtyFlags(bm);
    fixCount = getFixCounts(bm);

    // Print pool header with replacement strategy and total number of pages
    printf("{");
    printStrat(bm);
    printf(" %i}: ", bm->numPages);

    // Print each frame's content along with dirty and fix count flags
    for (i = 0; i < bm->numPages; i++)
        printf("%s[%i%s%i]", (i == 0 ? "" : ","), frameContent[i],
               (dirty[i] ? "x" : " "), fixCount[i]);
    printf("\n");

    // Free allocated arrays
    free(frameContent);
    free(dirty);
    free(fixCount);
}

/*
 * Function: sprintPoolContent
 * ---------------------------
 * Returns a string representation of the buffer pool's state.
 * The caller is responsible for freeing the returned string.
 */
char *sprintPoolContent(BM_BufferPool *const bm)
{
    PageNumber *frameContent;
    int *dirty;
    int *fixCount;
    int i;
    char *message;
    int pos = 0;

    // Allocate a buffer for the output message; size is estimated based on number of pages.
    message = (char *) malloc(256 + (22 * bm->numPages));
    frameContent = getFrameContents(bm);
    dirty = getDirtyFlags(bm);
    fixCount = getFixCounts(bm);

    // Build the formatted string with frame content information
    for (i = 0; i < bm->numPages; i++)
        pos += sprintf(message + pos, "%s[%i%s%i]",
                       (i == 0 ? "" : ","), frameContent[i],
                       (dirty[i] ? "x" : " "), fixCount[i]);

    // Free temporary arrays
    free(frameContent);
    free(dirty);
    free(fixCount);
    return message;
}

/*
 * Function: printPageContent
 * --------------------------
 * Prints a formatted hexadecimal representation of the content
 * of a page.
 *
 * Parameters:
 *   page - Pointer to the BM_PageHandle representing the page.
 */
void printPageContent(BM_PageHandle *const page)
{
    int i;
    // Print the header with the page number
    printf("[Page %i]\n", page->pageNum);

    // Print page content in hexadecimal format; print a space every 8 bytes
    // and a newline every 64 bytes for readability.
    for (i = 1; i <= PAGE_SIZE; i++)
        printf("%02X%s%s", page->data[i],
               (i % 8) ? "" : " ",
               (i % 64) ? "" : "\n");
}

/*
 * Function: sprintPageContent
 * ---------------------------
 * Returns a string with a formatted hexadecimal representation of the
 * page's content. The caller is responsible for freeing the returned string.
 */
char *sprintPageContent(BM_PageHandle *const page)
{
    int i;
    char *message;
    int pos = 0;

    // Allocate a buffer; size is estimated based on page size and formatting overhead
    message = (char *) malloc(30 + (2 * PAGE_SIZE) + (PAGE_SIZE % 64) + (PAGE_SIZE % 8));
    pos += sprintf(message + pos, "[Page %i]\n", page->pageNum);

    // Build the formatted string with page content in hexadecimal format
    for (i = 1; i <= PAGE_SIZE; i++)
        pos += sprintf(message + pos, "%02X%s%s", page->data[i],
                       (i % 8) ? "" : " ",
                       (i % 64) ? "" : "\n");

    return message;
}

/*
 * Function: printStrat
 * --------------------
 * Local helper function to print the buffer pool's replacement strategy.
 *
 * Parameters:
 *   bm - Pointer to the BM_BufferPool.
 */
void printStrat(BM_BufferPool *const bm)
{
    switch (bm->strategy)
    {
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
 * Statistics Interface Functions
 *
 * These functions return arrays of statistics for the buffer pool.
 */

/*
 * Function: getFrameContents
 * --------------------------
 * Returns an array of PageNumbers (of size numPages) where the ith element
 * is the number of the page stored in the ith frame. If a frame is empty,
 * it contains the constant NO_PAGE.
 *
 * Parameters:
 *   bm - Pointer to the BM_BufferPool.
 *
 * Returns:
 *   Dynamically allocated array of PageNumbers. The caller must free this array.
 */
PageNumber *getFrameContents(BM_BufferPool *const bm)
{
    if (bm == NULL) {
        return NULL;
    }

    int totalNumPages = bm->numPages;
    PageCache* pageCache = bm->mgmtData;
    PageNumber *arr = (PageNumber*) malloc(totalNumPages * sizeof(PageNumber));
    int i;
    for (i = 0; i < totalNumPages; i++) {
        arr[i] = pageCache->arr[i]->pageNum;
    }
    return arr;
}

/*
 * Function: getDirtyFlags
 * -----------------------
 * Returns an array of integers (of size numPages) where the ith element is
 * non-zero if the page in the ith frame is dirty; otherwise, it is 0.
 *
 * Parameters:
 *   bm - Pointer to the BM_BufferPool.
 *
 * Returns:
 *   Dynamically allocated array of integers indicating dirty flags.
 */
int *getDirtyFlags(BM_BufferPool *const bm)
{
    if (bm == NULL) {
        return NULL;
    }

    PageCache* pageCache = bm->mgmtData;
    int numPages = bm->numPages;
    int *arr = (int*) malloc(numPages * sizeof(int));
    int i;
    for (i = 0; i < numPages; i++) {
        arr[i] = pageCache->arr[i]->dirtyBit;
    }
    return arr;
}

/*
 * Function: getFixCounts
 * ----------------------
 * Returns an array of integers (of size numPages) where the ith element is the
 * fix count (pin count) of the page stored in the ith frame. For empty frames,
 * the value is 0.
 *
 * Parameters:
 *   bm - Pointer to the BM_BufferPool.
 *
 * Returns:
 *   Dynamically allocated array of integers indicating fix counts.
 */
int *getFixCounts(BM_BufferPool *const bm)
{
    if (bm == NULL) {
        return NULL;
    }

    PageCache* pageCache = bm->mgmtData;
    int numPages = bm->numPages;
    int *arr = (int*) malloc(numPages * sizeof(int));
    int i;
    for (i = 0; i < numPages; i++) {
        arr[i] = pageCache->arr[i]->pinCount;
    }
    return arr;
}

/*
 * Function: getNumReadIO
 * ----------------------
 * Returns the total number of read I/O operations performed by the buffer manager.
 *
 * Parameters:
 *   bm - Pointer to the BM_BufferPool.
 *
 * Returns:
 *   Number of read I/Os, or -1 if bm is NULL.
 */
int getNumReadIO(BM_BufferPool *const bm)
{
    if (bm == NULL) {
        return -1;
    }
    // Retrieve the number of read I/Os from the page cache
    PageCache* pageCache = bm->mgmtData;
    return pageCache->numRead;
}

/*
 * Function: getNumWriteIO
 * -----------------------
 * Returns the total number of write I/O operations performed by the buffer manager.
 *
 * Parameters:
 *   bm - Pointer to the BM_BufferPool.
 *
 * Returns:
 *   Number of write I/Os, or -1 if bm is NULL.
 */
int getNumWriteIO(BM_BufferPool *const bm)
{
    if (bm == NULL) {
        return -1;
    }
    // Retrieve the number of write I/Os from the page cache
    PageCache* pageCache = bm->mgmtData;
    return pageCache->numWrite;
}