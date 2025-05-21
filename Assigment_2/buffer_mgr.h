/************************************************************
*     File name:                buffer_mgr.h                *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/

#ifndef BUFFER_MANAGER_H // Include guard to prevent multiple inclusions
#define BUFFER_MANAGER_H

// Include return codes and methods for logging errors
#include "dberror.h" // Include the dberror.h header file for error handling

// Include bool DT
#include "dt.h" // Include the dt.h header file (likely for boolean type definition)
#include "storage_mgr.h" // Include the storage_mgr.h header file for storage manager functions and data structures

// Replacement Strategies (Enum defining the different page replacement strategies)
typedef enum ReplacementStrategy {
    RS_FIFO = 0, // First-In, First-Out
    RS_LRU = 1, // Least Recently Used
    RS_CLOCK = 2, // Clock (Not implemented in this assignment, but included for completeness)
    RS_LFU = 3, // Least Frequently Used (Not implemented)
    RS_LRU_K = 4 // LRU-K (A variation of LRU)
} ReplacementStrategy;

// Data Types and Structures
typedef int PageNumber; // Define PageNumber as an integer type
#define NO_PAGE -1      // Define NO_PAGE as -1, indicating that a frame is empty

// Structure representing a Buffer Pool
typedef struct BM_BufferPool {
    char *pageFile; // Name of the associated page file
    int numPages; // Number of pages (frames) in the buffer pool
    ReplacementStrategy strategy; // Page replacement strategy used by the buffer pool
    void *mgmtData; // Pointer to management data (bookkeeping information) for the buffer pool
} BM_BufferPool;

// Structure representing a Page Handle
typedef struct BM_PageHandle {
    PageNumber pageNum; // Page number associated with the page handle
    char *data; // Pointer to the page data in memory
} BM_PageHandle;


int findFrame(BM_BufferPool *const bm, PageNumber pageNum); // Function declaration for findFrame

// -----------------------------------------------------------------------------
// Data Structures
// -----------------------------------------------------------------------------

// Define Frame structure (Represents a single frame in the buffer pool)
typedef struct Frame {
    PageNumber pageNum; // Page number stored in this frame
    char *data; // Pointer to the actual page data (memory)
    bool dirty; // Flag indicating if the page has been modified (dirty = true)
    int fixCount; // Number of times the page is currently pinned (fixCount > 0 means the page can't be replaced)
    int loadTime; // Time the page was loaded into the frame (used for FIFO)
    int lastUsed; // Time the page was last accessed (used for LRU)
    int kthAccess; // Counter for LRU-K (not directly used but could be helpful)
    // For LRU-K (k=2): Store the timestamps of the last two accesses
    int lastTwo[2]; // Array to store the two most recent access timestamps for LRU-K
    int accessCount;
    int useBit;
} Frame;

// Define BM_MgmtData structure (Management data for the buffer pool)
typedef struct BM_MgmtData {
    SM_FileHandle fh; // File handle for the associated page file (from the Storage Manager)
    Frame *frames; // Array of frames in the buffer pool
    int numFrames; // Number of frames in the buffer pool
    int loadTimeCounter; // Counter for tracking load times (used in FIFO)
    int accessCounter; // Counter for tracking access times (used in LRU and LRU-K)
    int readIO; // Number of read I/O operations
    int writeIO; // Amount of writing I/O operations
    int k; // K value for LRU-K algorithm
    int clock_hand;
} BM_MgmtData;


// Convenience macros (Shorthand for allocating memory for the structures)
#define MAKE_POOL() ((BM_BufferPool *) malloc (sizeof(BM_BufferPool)))         // Creates a BM_BufferPool structure
#define MAKE_PAGE_HANDLE() ((BM_PageHandle *) malloc (sizeof(BM_PageHandle))) // Creates a BM_PageHandle structure

// Buffer Manager Interface - Pool Handling (Functions for managing the buffer pool)
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName,
                  const int numPages, ReplacementStrategy strategy,
                  void *stratData); // Initializes the buffer pool
RC shutdownBufferPool(BM_BufferPool *const bm); // Shuts down the buffer pool
RC forceFlushPool(BM_BufferPool *const bm); // Writes all dirty pages back to disk

// Buffer Manager Interface - Access Pages (Functions for accessing pages in the buffer pool)
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page); // Marks a page as dirty
RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page); // Unpins a page
RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page); // Writes a page to disk
RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page,
           const PageNumber pageNum); // Pins a page into the buffer pool

// Statistics Interface (Functions for retrieving statistics about the buffer pool)
PageNumber *getFrameContents(BM_BufferPool *const bm); // Gets an array of page numbers in each frame
bool *getDirtyFlags(BM_BufferPool *const bm); // Gets an array of dirty flags for each frame
int *getFixCounts(BM_BufferPool *const bm); // Gets an array of fix counts for each frame
int getNumReadIO(BM_BufferPool *const bm); // Gets the number of read I/O operations
int getNumWriteIO(BM_BufferPool *const bm); // Gets the number of write I/O operations

#endif // End of include guard
