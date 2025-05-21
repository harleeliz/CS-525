/************************************************************
 * File name:      buffer_mgr_stat.h
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This header file declares debug functions for the Buffer Manager.
 *   These functions allow printing or obtaining string representations
 *   of the current buffer pool and page contents, which are useful for
 *   debugging and performance analysis.
 ************************************************************/

#ifndef BUFFER_MGR_STAT_H
#define BUFFER_MGR_STAT_H

#include "buffer_mgr.h"  // Include the Buffer Manager interface

/*
 * Debug Functions for Buffer Manager Statistics:
 *
 * The following functions are used to display the content of the buffer pool
 * or a specific page. This is useful during development to inspect the state
 * of the buffer pool and verify that pages are being managed correctly.
 */

/*
 * Function: printPoolContent
 * --------------------------
 * Description:
 *   Prints the current content of the buffer pool to the standard output.
 *
 * Parameters:
 *   bm - Pointer to a BM_BufferPool structure representing the buffer pool.
 */
void printPoolContent(BM_BufferPool *const bm);

/*
 * Function: printPageContent
 * --------------------------
 * Description:
 *   Prints the content of a specific page to the standard output.
 *
 * Parameters:
 *   page - Pointer to a BM_PageHandle structure representing the page.
 */
void printPageContent(BM_PageHandle *const page);

/*
 * Function: sprintPoolContent
 * ---------------------------
 * Description:
 *   Returns a string representation of the current content of the buffer pool.
 *
 * Parameters:
 *   bm - Pointer to a BM_BufferPool structure representing the buffer pool.
 *
 * Returns:
 *   A pointer to a character string that contains the buffer pool content.
 *   The caller is responsible for freeing this string if memory is allocated.
 */
char *sprintPoolContent(BM_BufferPool *const bm);

/*
 * Function: sprintPageContent
 * ---------------------------
 * Description:
 *   Returns a string representation of the content of a specific page.
 *
 * Parameters:
 *   page - Pointer to a BM_PageHandle structure representing the page.
 *
 * Returns:
 *   A pointer to a character string that contains the page content.
 *   The caller is responsible for freeing this string if memory is allocated.
 */
char *sprintPageContent(BM_PageHandle *const page);

#endif