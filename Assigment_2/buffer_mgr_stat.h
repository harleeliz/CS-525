/************************************************************
*     File name:                   buffer_mgr_stat.h        *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/

#ifndef BUFFER_MGR_STAT_H  // Include guard to prevent multiple inclusions of this header file
#define BUFFER_MGR_STAT_H

#include "buffer_mgr.h" // Include the buffer manager header file

// Debug functions (These functions are likely for debugging and testing purposes)

/*
 * Prints the contents of the buffer pool to standard output.
 * This function is likely used for debugging to inspect the state of the buffer pool.
 *
 * @param bm The buffer pool to print.
 */
void printPoolContent(BM_BufferPool *const bm);

/*
 * Prints the content of a page to standard output.
 * This function is likely used for debugging to inspect the data within a specific page.
 *
 * @param page The page handle representing the page to print.
 */
void printPageContent(BM_PageHandle *const page);

/*
 * Returns a string representation of the contents of the buffer pool.
 * This function allows capturing the buffer pool's state as a string, potentially for logging or testing.
 * The caller is responsible for freeing the memory allocated for the returned string.
 *
 * @param bm The buffer pool to represent as a string.
 * @return A dynamically allocated string representing the buffer pool's contents.
 */
char *sprintPoolContent(BM_BufferPool *const bm);

/*
 * Returns a string representation of the content of a page.
 *  This function allows capturing the page's data as a string, potentially for logging or testing.
 * The caller is responsible for freeing the memory allocated for the returned string.
 *
 * @param page The page handle representing the page to represent as a string.
 * @return A dynamically allocated string representing the page's content.
 */
char *sprintPageContent(BM_PageHandle *const page);

#endif // End of include guard
