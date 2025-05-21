/************************************************************
 * File name: buffer_mgr.h
 * Course: CS 525 Advanced Database Organization (Spring 2025)
 * Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This header file defines the Buffer Manager interface,
 *   data types, structures, and helper functions used for
 *   managing the buffer pool and page cache.
 ************************************************************/

#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

/*------------------------------------------------------------
 * Included header files:
 *-----------------------------------------------------------*/
#include "dberror.h"
#include "storage_mgr.h"
#include "dt.h"
#include <stdbool.h>
#include <stdlib.h>

/*------------------------------------------------------------
 * Replacement Strategies
 *-----------------------------------------------------------*/
typedef enum ReplacementStrategy {
    RS_FIFO = 0,
    RS_LRU = 1,
    RS_CLOCK = 2,
    RS_LFU = 3,
    RS_LRU_K = 4
} ReplacementStrategy;

/*------------------------------------------------------------
 * Basic Data Types and Constants
 *-----------------------------------------------------------*/
typedef int PageNumber;
#define NO_PAGE -1

/*------------------------------------------------------------
 * Buffer Pool Data Structure
 *-----------------------------------------------------------*/
typedef struct BM_BufferPool {
    char *pageFile;             // Name of the page file associated with the buffer pool
    int numPages;               // Number of page frames in the buffer pool
    ReplacementStrategy strategy; // Replacement strategy to use
    void *mgmtData;             // Pointer to internal management data for the buffer manager
} BM_BufferPool;

/*------------------------------------------------------------
 * Page Handle Data Structure
 *-----------------------------------------------------------*/
typedef struct BM_PageHandle {
    PageNumber pageNum;  // Page number in the page file
    char *data;          // Pointer to the memory storing the page content
} BM_PageHandle;

/*------------------------------------------------------------
 * Frame Structure (Internal)
 *-----------------------------------------------------------*/
typedef struct Frame {
    PageNumber pageNum;      // The page number stored in this frame
    char *data;              // Pointer to page data (size = PAGE_SIZE)
    bool dirty;              // True if page has been modified in memory
    int fixCount;            // Number of clients that have pinned this page
    int lastTwo[2];          // For FIFO/LRU: stores load time or recent access times
    int accessCount;         // For LFU: counts the number of accesses
    int useBit;              // For CLOCK: 0 or 1
} Frame;

/*------------------------------------------------------------
 * BM_MgmtData (Internal Management Data)
 *-----------------------------------------------------------*/
typedef struct BM_MgmtData {
    SM_FileHandle fh;       // Underlying file handle from storage_mgr
    int numFrames;          // Total number of frames in the buffer pool
    Frame *frames;          // Array of frames
    int readIO;             // Total number of disk read operations
    int writeIO;            // Total number of disk write operations
    int accessCounter;      // Global counter used for replacement strategies
    int loadTimeCounter;    // Not used if using accessCounter
    int clock_hand;         // Index used by CLOCK strategy
} BM_MgmtData;

/*------------------------------------------------------------
 * Hash Structure for LRU (Internal)
 *-----------------------------------------------------------*/
typedef struct Hash {
    int capacity;        // Maximum capacity of the hash table
    int pageCnt;         // Current number of pages stored
    Frame **arr;         // Array of pointers to frames
} Hash;

/*------------------------------------------------------------
 * Page Cache Structure (Internal)
 *-----------------------------------------------------------*/
typedef struct PageCache {
    int front;          // Index of the front element in the page cache (queue)
    int rear;           // Index of the rear element in the page cache (queue)
    int frameCnt;       // Number of used frames in the cache
    int capacity;       // Total capacity of the cache
    Frame **arr;        // Array of pointers to frames
    int numRead;        // Number of pages read into the cache
    int numWrite;       // Number of pages written from the cache
    SM_FileHandle *fHandle; // File handle to the associated page file
    int *hash;          // Auxiliary array for quick look-up in LRU implementation
} PageCache;

/*------------------------------------------------------------
 * Convenience Macros
 *-----------------------------------------------------------*/
#define MAKE_POOL()           ((BM_BufferPool *) malloc(sizeof(BM_BufferPool)))
#define MAKE_PAGE_HANDLE()    ((BM_PageHandle *) malloc(sizeof(BM_PageHandle)))

/*------------------------------------------------------------
 * Helper Interface (Internal)
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
 * Page Cache Management Functions (Internal)
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
 *-----------------------------------------------------------*/
extern RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                         const int numPages, ReplacementStrategy strategy, void *stratData);
extern RC shutdownBufferPool(BM_BufferPool *const bm);
extern RC forceFlushPool(BM_BufferPool *const bm);

/*------------------------------------------------------------
 * Buffer Manager Interface: Access Pages
 *-----------------------------------------------------------*/
extern RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page);
extern RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page);
extern RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page);
extern RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page, const PageNumber pageNum);

/*------------------------------------------------------------
 * Buffer Manager Statistics Interface
 *-----------------------------------------------------------*/
extern PageNumber *getFrameContents(BM_BufferPool *const bm);
extern bool *getDirtyFlags(BM_BufferPool *const bm);
extern int *getFixCounts(BM_BufferPool *const bm);
extern int getNumReadIO(BM_BufferPool *const bm);
extern int getNumWriteIO(BM_BufferPool *const bm);

#endif /* BUFFER_MANAGER_H */
