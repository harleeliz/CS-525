/************************************************************
 * File name:      buffer_mgr_stat.h
 * CS 525 Advanced Database Organization (Spring 2025)
 * Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/
#ifndef BUFFER_MGR_STAT_H
#define BUFFER_MGR_STAT_H
#include "buffer_mgr.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Debug Functions */

/*
 * Prints the contents of the buffer pool to standard output.
 * Used for debugging to inspect the state of the buffer pool.
 *
 * @param bm: The buffer pool to print.
 */
void printPoolContent(BM_BufferPool *const bm);

/*
 * Prints the content of a page to standard output.
 * Used for debugging to inspect the data within a specific page.
 *
 * @param page: The page handle representing the page to print.
 */
void printPageContent(BM_PageHandle *const page);

/*
 * Returns a string representation of the contents of the buffer pool.
 * The caller is responsible for freeing the allocated memory.
 *
 * @param bm: The buffer pool.
 * @return: A dynamically allocated string representing the buffer pool's contents.
 */
char *sprintPoolContent(BM_BufferPool *const bm);

/*
 * Returns a string representation of the content of a page.
 * The caller is responsible for freeing the allocated memory.
 *
 * @param page: The page handle representing the page.
 * @return: A dynamically allocated string representing the page's content.
 */
char *sprintPageContent(BM_PageHandle *const page);

/* Statistics Functions */

/*
 * Returns an array containing the page numbers stored in each frame of the buffer pool.
 * For empty frames, the value is NO_PAGE.
 * The caller must free the returned array.
 */
PageNumber *getFrameContents(BM_BufferPool *const bm);

/*
 * Returns an array indicating the dirty status of each frame in the buffer pool.
 * The caller must free the allocated memory.
 */
bool *getDirtyFlags(BM_BufferPool *const bm);

/*
 * Returns an array containing the fix counts for each frame in the buffer pool.
 * The caller must free the allocated memory.
 */
int *getFixCounts(BM_BufferPool *const bm);

/*
 * Returns the total number of read I/O operations performed on the buffer pool.
 */
int getNumReadIO(BM_BufferPool *const bm);

/*
 * Returns the total number of write I/O operations performed on the buffer pool.
 */
int getNumWriteIO(BM_BufferPool *const bm);

#ifdef __cplusplus
}
#endif

#endif // BUFFER_MGR_STAT_H