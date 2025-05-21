# Assignment 2: Buffer Manager Implementation

### CS 525 Advanced Database Organization (Spring 2025)

Professor: Gerald Balekaki\
TAs: Mohammadreza Sediqin, Nency Patel, Rashmi Topannavar, Ramani Chinthakindi\
Students: Harlee Ramos, Jisun Yun, Baozhu Xie

---

## **1. Introduction**

This project implements a **buffer manager** for managing pages in memory using different replacement strategies, including **FIFO (First-In, First-Out)** and **LRU (Least Recently Used)**. The buffer manager interacts with a **storage manager** (developed in Assignment 1) to efficiently load and replace pages from disk.

### **Key Features:**

- **Buffer Pool Management**: Initialize, shutdown, and manage multiple buffer pools.
- **Page Replacement Strategies**: Supports FIFO, LRU, and LRU-K.
- **Page Pinning and Unpinning**: Controls access to memory pages.
- **Dirty Page Handling**: Tracks modified pages and writes them to disk when necessary.
- **Performance Tracking**: Monitors read and write I/O operations.

---

## **2. Code Structure**

The project is structured as follows:

| File Name           | Description                                                        |
| ------------------- | ------------------------------------------------------------------ |
| `buffer_mgr.c`      | Implements buffer pool management and page replacement strategies. |
| `buffer_mgr.h`      | Declares buffer manager functions and structures.                  |
| `storage_mgr.c`     | Implements file-based storage operations.                          |
| `storage_mgr.h`     | Declares storage manager functions and data structures.            |
| `buffer_mgr_stat.c` | Provides functions to print buffer pool statistics.                |
| `buffer_mgr_stat.h` | Declares debugging and statistics functions.                       |
| `dberror.c`         | Implements error handling functions.                               |
| `dberror.h`         | Defines error codes and error message functions.                   |
| `dt.h`              | Defines boolean data types for compatibility.                      |
| `test_assign2_1.c`  | Contains test cases for buffer manager functionality (FIFO & LRU). |
| `test_assign2_2.c`  | Contains additional test cases for LRU-K and error handling.       |
| `test_helper.h`     | Provides macros and utilities for testing.                         |
| `makefile`          | Builds the project and generates test binaries.                    |
| `README.md`         | Documentation of the project and implementation details.           |

---

## **3. Buffer Manager Functions**

### **3.1. Initialization**

- `initBufferPool(BM_BufferPool *bm, const char *pageFileName, int numPages, ReplacementStrategy strategy, void *stratData)`: Initializes a buffer pool with a given size and page replacement strategy.
- `shutdownBufferPool(BM_BufferPool *bm)`: Shuts down the buffer pool, writing back dirty pages to disk.
- `forceFlushPool(BM_BufferPool *bm)`: Forces all dirty pages to be written to disk.
- `void replacePage(BM_BufferPool *bm, int frameIndex)`: This function resets a frame in the buffer pool, setting its page number to NO_PAGE, marking it as not dirty, and resetting its fix count.

### **3.2. Page Management**

- `pinPage(BM_BufferPool *bm, BM_PageHandle *page, PageNumber pageNum)`: Pins a page in the buffer pool, loading it from disk if necessary.
- `unpinPage(BM_BufferPool *bm, BM_PageHandle *page)`: Unpins a page, allowing it to be replaced if necessary.
- `markDirty(BM_BufferPool *bm, BM_PageHandle *page)`: Marks a page as modified (dirty).
- `forcePage(BM_BufferPool *bm, BM_PageHandle *page)`: Forces a specific page to be written back to disk.

### **3.3. Statistics & Debugging**

- `getFrameContents(BM_BufferPool *bm)`: Returns an array of page numbers in the buffer pool.
- `getDirtyFlags(BM_BufferPool *bm)`: Returns an array indicating which pages are dirty.
- `getFixCounts(BM_BufferPool *bm)`: Returns an array of fix counts for each page in the buffer pool.
- `getNumReadIO(BM_BufferPool *bm)`: Returns the number of reads I/O operations performed.
- `getNumWriteIO(BM_BufferPool *bm)`: Returns the number of write I/O operations performed.

---

## **4. Page Replacement Strategies**

The buffer manager supports multiple **page replacement strategies**:

### **4.1. FIFO (First-In, First-Out)**

- Replace the oldest page in the buffer pool.
- Implemented using a load timestamp for each page.

### **4.2. LRU**

- Replaces the least recently accessed page.
- Implemented by tracking the last accessed timestamp.

### **4.3. LRU-K (Least Recently Used-K)** 

- Tracks the **K-th** most recent access to make replacement decisions.
- More accurate than traditional LRU but requires more memory.

### **4.4. RS_CLOCK**
- It uses a circular pointer (clock_hand) to find pages for replacement. A use-bit is checked and reset if necessary before selecting a victim.

### **4.5. LFU**
- Tracks how frequently pages are accessed `accessCount` and replaces the least frequently used page.
---

## **5. Error Handling**

- Errors are defined in `dberror.h` and reported using `printError()`.
- Common error codes include:
    - `RC_FILE_NOT_FOUND`: Page file does not exist.
    - `RC_NO_FREE_BUFFER_ERROR`: No available frames in the buffer.
    - `RC_PINNED_PAGES_IN_BUFFER`: Attempt to shut down with pinned pages.

---

## **6. Testing and Validation**

The project includes test cases to validate the buffer manager's functionality:

- **Test File: **``
    - Tests FIFO and LRU strategies with various page access patterns.

- **Test File: **``
    - Tests LRU-K strategy and error handling scenarios.
- Additional test cases for CLOCK and LFU strategies on test_assign2_2.

**Example Test Case:** *(Checking buffer pool contents after multiple page requests)*

```c
const char *poolContents[] = {
    "[0 0],[-1 0],[-1 0]",
    "[0 0],[1 0],[-1 0]",
    "[0 0],[1 0],[2 0]",
    "[3 0],[1 0],[2 0]",
};
```

---

## **7. Build and Execution Instructions**

1. **Navigate to the project directory:**
   ```sh
   cd /path/to/assign2
   ```
2. **Compile the project:**
   ```sh
   make all
   ```
3. **Run tests:**
   ```sh
   ./test_assign2_1
   ./test_assign2_2
   ```
4. **Clean compiled files:**
   ```sh
   make clean
   ```

---

## **8. Memory Management Considerations**

- All dynamically allocated memory is freed properly using `free()`.
- The `shutdownBufferPool()` function ensures that resources are released before exiting.

---

## **9. Optional Extensions**

- Implement **thread safety** for buffer pool operations.
- Extend **additional page replacement strategies** (CLOCK, LFU).





