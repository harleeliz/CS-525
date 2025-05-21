/************************************************************
 * File name: buffer_mgr.h
 * Course: CS 525 Advanced Database Organization (Spring 2025)
 * Authors: Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This header file defines the Buffer Manager interface,
 *   data types, structures, and helper functions used for
 *   managing the buffer pool and page cache. The file includes
 *   declarations for various page replacement strategies, page
 *   and buffer pool data structures, and interface functions
 *   for both buffer management and statistics.
 ************************************************************/

#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

/*------------------------------------------------------------
 * Included header files:
 * - dberror.h: Definitions for return codes and error logging.
 * - storage_mgr.h: Storage Manager interface for file operations.
 * - dt.h: Defines the boolean data type.
 *-----------------------------------------------------------*/
#include "dberror.h"
#include "storage_mgr.h"
#include "dt.h"

/*------------------------------------------------------------
 * Replacement Strategies:
 * Defines various strategies for replacing pages in the buffer pool.
 * RS_FIFO  : First-In-First-Out replacement strategy.
 * RS_LRU   : Least Recently Used replacement strategy.
 * RS_CLOCK : Clock (second chance) replacement strategy.
 * RS_LFU   : Least Frequently Used replacement strategy.
 * RS_LRU_K : A variant of LRU taking K previous references into account.
 *-----------------------------------------------------------*/
typedef enum ReplacementStrategy {
	RS_FIFO = 0,
	RS_LRU = 1,
	RS_CLOCK = 2,
	RS_LFU = 3,
	RS_LRU_K = 4
} ReplacementStrategy;

/*------------------------------------------------------------
 * Basic Data Types and Constants:
 * - PageNumber: Defined as an integer to represent a page's number.
 * - NO_PAGE   : Constant used to denote that no page is available.
 *-----------------------------------------------------------*/
typedef int PageNumber;
#define NO_PAGE -1

/*------------------------------------------------------------
 * Buffer Pool Data Structure:
 * BM_BufferPool encapsulates the information for a buffer pool.
 *
 * Members:
 *  - pageFile: Name of the file backing the pages.
 *  - numPages: Total number of page frames in the pool.
 *  - strategy: Replacement strategy to use (e.g., FIFO, LRU, etc.).
 *  - mgmtData: Pointer for internal management data specific to the pool.
 *-----------------------------------------------------------*/
typedef struct BM_BufferPool {
	char *pageFile;           // Name of the page file associated with the buffer pool
	int numPages;             // Number of page frames in the buffer pool
	ReplacementStrategy strategy; // Page replacement strategy in use
	void *mgmtData;           // Pointer to internal bookkeeping data for the buffer manager
} BM_BufferPool;

/*------------------------------------------------------------
 * Page Handle Data Structure:
 * BM_PageHandle represents a handle for accessing page data.
 *
 * Members:
 *  - pageNum: The page's position in the page file (starting at 0).
 *  - data: Pointer to the memory area storing the page's content.
 *-----------------------------------------------------------*/
typedef struct BM_PageHandle {
	PageNumber pageNum;  // Position of the page in the page file
	char *data;          // Pointer to the memory storing the page content
} BM_PageHandle;

/*------------------------------------------------------------
 * Frame Structure:
 * Each frame in the buffer pool represents a cached page.
 *
 * Members:
 *  - pageNum: Indicates which page is stored in the frame.
 *  - pinCount: Number of clients currently using this page.
 *  - dirtyBit: Flag indicating if the page has been modified.
 *  - data: Pointer to the memory area that stores the page content.
 *-----------------------------------------------------------*/
typedef struct Frame {
	PageNumber pageNum;  // Page number currently stored in the frame
	int pinCount;        // Number of processes using this page
	int dirtyBit;        // Indicates if the page has been modified (dirty)
	char* data;          // Pointer to the content stored in the frame
} Frame;

/*------------------------------------------------------------
 * Hash Structure for LRU Implementation:
 * Used for quickly mapping page numbers to frames in the cache.
 *
 * Members:
 *  - capacity: Total capacity of the hash table.
 *  - pageCnt: Current count of pages stored.
 *  - arr: Array of pointers to Frame structures.
 *-----------------------------------------------------------*/
typedef struct Hash {
    int capacity;        // Maximum capacity of the hash table
	int pageCnt;         // Current number of pages in the hash table
    Frame* *arr;         // Array of pointers to frames
} Hash;

/*------------------------------------------------------------
 * Page Cache Structure:
 * Represents the cached pages in the buffer pool.
 *
 * Members:
 *  - front, rear: Pointers for managing the cache as a queue.
 *  - frameCnt: Number of frames currently used in the cache.
 *  - capacity: Total capacity (i.e., maximum number of frames).
 *  - arr: Array storing pointers to frames' information.
 *  - numRead: Total number of pages read into the cache.
 *  - numWrite: Total number of pages written from the cache.
 *  - fHandle: File handle for the associated page file.
 *  - hash: Auxiliary array for hash mapping (used by LRU).
 *-----------------------------------------------------------*/
typedef struct PageCache {
	int front;          // Index of the front element in the page cache (queue)
	int rear;           // Index of the rear element in the page cache (queue)
	int frameCnt;       // Number of used frames in this cache
	int capacity;       // Total capacity of frames that can be stored
	Frame* *arr;        // Array of pointers to frame information
	int numRead;        // Counter for the number of pages read
	int numWrite;       // Counter for the number of pages written
	SM_FileHandle* fHandle; // File handle to manage the associated file
	int* hash;          // Hash array for quick look-up in LRU implementation
} PageCache;

/*------------------------------------------------------------
 * Convenience Macros:
 * - MAKE_POOL: Allocates memory for a BM_BufferPool structure.
 * - MAKE_PAGE_HANDLE: Allocates memory for a BM_PageHandle structure.
 *-----------------------------------------------------------*/
#define MAKE_POOL()           ((BM_BufferPool *) malloc(sizeof(BM_BufferPool)))
#define MAKE_PAGE_HANDLE()    ((BM_PageHandle *) malloc(sizeof(BM_PageHandle)))

/*------------------------------------------------------------
 * Helper Interface:
 * Functions to create, reset, and free various resources used in
 * managing the buffer pool and page cache.
 *-----------------------------------------------------------*/
extern Frame* createFrameNode();
extern RC resetFrameNode(Frame* frame);
extern int* createHash(int capacity);
extern PageCache* createPageCache(BM_BufferPool *const bm, int numPages);
extern void freeFrame(PageCache* pageCache);
extern void freeFileHandle(PageCache* pageCache);
extern void freeHash(PageCache* pageCache);
extern void freePageCache(PageCache* pageCache);

/*------------------------------------------------------------
 * Page Cache Management Functions:
 * Functions for checking cache state, adding pages using various
 * replacement strategies (FIFO, LRU), updating the cache, and
 * searching for pages within the cache.
 *-----------------------------------------------------------*/
extern int isFull(PageCache* pageCache);
extern int isEmpty(PageCache* pageCache);
extern Frame* isHitPageCache(PageCache* pageCache, const PageNumber pageNum);
extern RC addPageToPageCacheWithFIFO(BM_BufferPool *const bm, BM_PageHandle *const page, int pageNum);
extern RC addPageToPageCacheWithLRU(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum);
extern RC updateLRUOrder(PageCache* pageCache, int pageNum);
extern RC removePageWithFIFO(BM_BufferPool *const bm, BM_PageHandle *const page);
extern Frame* removePageWithLRU(BM_BufferPool *const bm, BM_PageHandle *const page, int leastUsedPage);
extern Frame* searchPageFromCache(PageCache *const pageCache, int pageNum);

/*------------------------------------------------------------
 * Buffer Manager Interface: Pool Handling
 * Functions to initialize, shutdown, and force flush the buffer pool.
 *-----------------------------------------------------------*/
extern RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
 		const int numPages, ReplacementStrategy strategy, void *stratData);
extern RC shutdownBufferPool(BM_BufferPool *const bm);
extern RC forceFlushPool(BM_BufferPool *const bm);

/*------------------------------------------------------------
 * Buffer Manager Interface: Access Pages
 * Functions to mark a page as dirty, unpin a page, force a page
 * to disk, and pin a page into the buffer pool.
 *-----------------------------------------------------------*/
extern RC markDirty (BM_BufferPool *const bm, BM_PageHandle *const page);
extern RC unpinPage (BM_BufferPool *const bm, BM_PageHandle *const page);
extern RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page);
extern RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum);

/*------------------------------------------------------------
 * Buffer Manager Statistics Interface:
 * Functions to retrieve buffer pool statistics such as frame contents,
 * dirty flags, fix counts, and I/O operation counts.
 *-----------------------------------------------------------*/
extern PageNumber *getFrameContents (BM_BufferPool *const bm);
extern int *getDirtyFlags (BM_BufferPool *const bm);
extern int *getFixCounts (BM_BufferPool *const bm);
extern int getNumReadIO (BM_BufferPool *const bm);
extern int getNumWriteIO (BM_BufferPool *const bm);

#endif /* BUFFER_MANAGER_H */