/************************************************************
*     File name:                buffer_mgr.c                *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/

#include "buffer_mgr.h" // Include buffer manager header file
#include "storage_mgr.h" // Include storage manager header file
#include "dberror.h"    // Include error handling header file
#include <limits.h>   // Include limits.h for INT_MAX
#include <stdlib.h>   // Include stdlib.h for malloc, free, etc.
#include <stdio.h>    // Include stdio.h for printf
#include <string.h>   // Include string.h for memset

// Function declarations
void debugPrintPageHistory(BM_BufferPool *const bm, const char *message);

int findFrame(BM_BufferPool *const bm, PageNumber pageNum);

static int selectVictim(BM_BufferPool *const bm);

void debugPrintAccess(const char *action, PageNumber pageNum, int accessCounter);

static int selectLRUVictim(BM_BufferPool *const bm);

// -----------------------------------------------------------------------------
// Helper Functions
// -----------------------------------------------------------------------------

/*
 * Replaces the page in the specified frame with an empty page (NO_PAGE).
 * This function essentially resets the frame, making it available for new pages to be loaded.
 * It is typically used as part of a page replacement strategy when a victim frame needs to be evicted.
 *
 * @param bm The buffer pool.
 * @param frameIndex The index of the frame to replace.
 */
void replacePage(BM_BufferPool *bm, int frameIndex) {
    BM_MgmtData *mgmt = (BM_MgmtData *) bm->mgmtData; // Get a pointer to the management data

    mgmt->frames[frameIndex].pageNum = NO_PAGE;
    // Set the page number to NO_PAGE, indicating that the frame is now empty.
    mgmt->frames[frameIndex].dirty = false;
    // Reset the dirty flag to false, as the frame no longer contains a modified page.
    mgmt->frames[frameIndex].fixCount = 0; // Reset the fix count to 0, as the frame is no longer pinned.
}


/*
 * Finds the index of the frame containing the given page number.
 *
 * @param bm The buffer pool.
 * @param pageNum The page number to search for.
 * @return The index of the frame containing the page number, or -1 if not found.
 */
int findFrame(BM_BufferPool *const bm, PageNumber pageNum) {
    BM_MgmtData *mgmtData = (BM_MgmtData *) bm->mgmtData; // Get management data
    for (int i = 0; i < mgmtData->numFrames; i++) {
        if (mgmtData->frames[i].pageNum == pageNum) {
            return i; // Found the frame
        }
    }
    return -1; // The Page isn't found in any frame
}

/*
 * Selects a victim frame based on the buffer pool's replacement strategy.
 *
 * @param bm The buffer pool.
 * @return The index of the selected victim frame.
 */
static int selectVictim(BM_BufferPool *const bm) {
    BM_MgmtData *mgmt = (BM_MgmtData *) bm->mgmtData; // Get a pointer to the management data
    int oldestFrame = 0; // Initialize oldestFrame (used for FIFO)
    int oldestLoadTime = INT_MAX; // Initialize oldestLoadTime (used for FIFO)

    // First check for empty frames.  If an empty frame (pageNum == NO_PAGE and fixCount == 0) is found, it's the easiest choice.
    for (int i = 0; i < mgmt->numFrames; i++) {
        if (mgmt->frames[i].pageNum == NO_PAGE && mgmt->frames[i].fixCount == 0) {
            printf("Found empty frame %d\n", i); // Print debug information (if enabled)
            return i; // Return the index of the empty frame
        }
    }

    // If no empty frames are found, select a victim based on the chosen replacement strategy.
    switch (bm->strategy) {
        case RS_FIFO: // First-In, First-Out replacement strategy
            // Find the frame with the oldest load time (the one that was loaded first).
            for (int i = 0; i < mgmt->numFrames; i++) {
                if (mgmt->frames[i].fixCount == 0) {
                    // Only consider frames that are not pinned
                    if (mgmt->frames[i].lastTwo[0] < oldestLoadTime) {
                        // Compare using the first element of lastTwo, which stores the oldest access time.
                        oldestLoadTime = mgmt->frames[i].lastTwo[0]; // Update the oldest load time
                        oldestFrame = i; // Update the index of the oldest frame
                    }
                }
            }
            return oldestFrame; // Return the index of the oldest frame (the FIFO victim)

        case RS_LRU: // Least Recently Used replacement strategy
            return selectLRUVictim(bm); // Call the helper function to select the LRU victim

        case RS_LRU_K: // LRU-K replacement strategy (k=2 in this implementation)
            printf("\nLRU-K victim selection:\n"); // Print debug information
            int victim = -1; // Initialize the victim index
            int oldestRecentAccess = INT_MAX; // Initialize the oldest recent access time

        // Print current state of frames (for debugging)
            for (int i = 0; i < mgmt->numFrames; i++) {
                printf("Frame %d: Page %d, History[%d, %d]\n",
                       i, mgmt->frames[i].pageNum,
                       mgmt->frames[i].lastTwo[0],
                       mgmt->frames[i].lastTwo[1]);
            }

        // Choose the victim based on the most recent access time (the second element of lastTwo).
            for (int i = 0; i < mgmt->numFrames; i++) {
                if (mgmt->frames[i].fixCount == 0) {
                    // Only consider frames that are not pinned
                    int mostRecentAccess = mgmt->frames[i].lastTwo[1]; // Get the most recent access time
                    if (mostRecentAccess < oldestRecentAccess) {
                        // If this is older than the currently oldest
                        oldestRecentAccess = mostRecentAccess; // Update the oldest recent access time
                        victim = i; // Update the victim candidate
                        printf("New victim candidate: frame %d (most recent access: %d)\n",
                               i, mostRecentAccess); // Print debug information
                    }
                }
            }

            printf("Final victim selection: frame %d\n", victim); // Print debug information
            return victim; // Return the index of the selected victim

        case RS_CLOCK: {
            // Clock replacement strategy
            int victimIndex = -1; // Initialize victimIndex

            // Loop through the frames using the clock hand
            while (true) {
                if (mgmt->frames[mgmt->clock_hand].fixCount == 0) {
                    // If the frame is not pinned
                    if (mgmt->frames[mgmt->clock_hand].useBit == 0) {
                        // Check the use bit
                        victimIndex = mgmt->clock_hand; // Select this frame as the victim
                        replacePage(bm, victimIndex); // Replace the page in the victim frame
                        break; // Exit the loop
                    } else {
                        mgmt->frames[mgmt->clock_hand].useBit = 0; // Reset the use bit
                    }
                }
                mgmt->clock_hand = (mgmt->clock_hand + 1) % bm->numPages; // Move the clock hand
            }
            return victimIndex; // Return the index of the selected victim
        }
        case RS_LFU: {
            // Least Frequently Used replacement strategy
            int minIndex = -1; // Initialize minIndex
            int minCount = INT_MAX; // Initialize minCount

            // Find the frame with the minimum access count
            for (int i = 0; i < bm->numPages; i++) {
                if (mgmt->frames[i].fixCount == 0 && mgmt->frames[i].accessCount < minCount) {
                    minCount = mgmt->frames[i].accessCount; // Update the minimum access count
                    minIndex = i; // Update the index of the frame with the minimum access count
                }
            }
            if (minIndex != -1) {
                // If a victim frame was found
                replacePage(bm, minIndex); // Replace the page in the victim frame
                return minIndex; // Return the victim frame index
            }
            return -1; // Return -1 if no victim was found
        }

        default:
            return 0; // Default case (no other strategies implemented)
    }
}

/*
 * Initializes the buffer pool.
 *
 * @param bm The buffer pool to initialize.
 * @param pageFileName The name of the page file.
 * @param numPages The number of pages in the buffer pool.
 * @param strategy The page replacement strategy.
 * @param stratData Strategy-specific data (not used in this implementation).
 * @return RC_OK on success, or an error code on failure.
 */
RC initBufferPool(BM_BufferPool *const bm,
                  const char *const pageFileName,
                  const int numPages,
                  ReplacementStrategy strategy,
                  void *stratData) {
    // 1. Open the page file.
    SM_FileHandle fh;
    if (openPageFile((char *) pageFileName, &fh) != RC_OK) {
        return RC_FILE_NOT_FOUND; // Return error if the file not found
    }

    // 2. Allocate and initialize management data for the buffer pool.
    BM_MgmtData *mgmt = malloc(sizeof(BM_MgmtData)); // Allocate memory for the management data structure
    mgmt->numFrames = numPages; // Set the number of frames in the buffer pool
    mgmt->fh = fh; // Store the file handle in the management data
    mgmt->readIO = 0; // Initialize the read I/O counter to 0
    mgmt->writeIO = 0; // Initialize the writing I/O counter to 0
    mgmt->accessCounter = 0; // Initialize the access counter (used for LRU) to 0
    mgmt->loadTimeCounter = 0; // Initialize the load time counter (used for FIFO) to 0
    mgmt->clock_hand = 0; // Initialize the clock hand (used for Clock strategy) to 0

    // 3. Initialize the individual frames in the buffer pool.
    mgmt->frames = malloc(sizeof(Frame) * numPages); // Allocate memory for the array of frames
    if (mgmt->frames == NULL) {
        return RC_MALLOC_FAILED; // Return error if memory allocation fails
    }
    for (int i = 0; i < numPages; i++) {
        mgmt->frames[i].pageNum = NO_PAGE; // Initialize the page number in each frame to NO_PAGE (empty)
        mgmt->frames[i].dirty = false; // Initialize the dirty flag in each frame to false (not modified)
        mgmt->frames[i].fixCount = 0; // Initialize the fix count in each frame to 0 (not pinned)
        mgmt->frames[i].data = malloc(PAGE_SIZE); // Allocate memory for the page data in each frame
        mgmt->frames[i].lastTwo[0] = 0; // Initialize the first last access time for LRU-K to 0
        mgmt->frames[i].lastTwo[1] = 0; // Initialize the second last access time for LRU-K to 0
        mgmt->frames[i].accessCount = 0; // Initialize the access count for LFU to 0
        mgmt->frames[i].useBit = 0; // Initialize the use bit for Clock to 0
    }

    // 4. Set up the fields of the BM_BufferPool structure.
    bm->pageFile = (char *) pageFileName; // Set the page file name
    bm->numPages = numPages; // Set the number of pages in the buffer pool
    bm->strategy = strategy; // Set the page replacement strategy
    bm->mgmtData = mgmt; // Set the management data pointer

    return RC_OK; // Return RC_OK to indicate successful initialization
}

/*
 * Shuts down the buffer pool.
 *
 * @param bm The buffer pool to shut down.
 * @return RC_OK on success, or an error code on failure.
 */
RC shutdownBufferPool(BM_BufferPool *const bm) {
    if (bm->mgmtData == NULL) {
        return RC_BUFFER_POOL_NOT_INIT; // or another appropriate error code
    }

    BM_MgmtData *mgmt = (BM_MgmtData *) bm->mgmtData;

    // 1. Check for any pinned pages.
    for (int i = 0; i < bm->numPages; i++) {
        if (mgmt->frames[i].fixCount > 0) {
            return RC_PINNED_PAGES_IN_BUFFER; // Cannot shut down with pinned pages
        }
    }

    // 2. Flush any dirty pages.
    RC rc = forceFlushPool(bm);
    if (rc != RC_OK) {
        return rc;
    }

    // 3. Free memory.
    for (int i = 0; i < bm->numPages; i++) {
        free(mgmt->frames[i].data); // Free page data
    }
    free(mgmt->frames); // Free frames array
    closePageFile(&mgmt->fh); // Close the page file
    free(mgmt); // Free management data

    bm->mgmtData = NULL; // Reset buffer pool data
    bm->pageFile = NULL;

    return RC_OK;
}


/*
 * Forces all dirty pages in the buffer pool to disk.
 *
 * @param bm The buffer pool.
 * @return RC_OK on success.
 */
RC forceFlushPool(BM_BufferPool *const bm) {
    BM_MgmtData *mgmt = (BM_MgmtData *) bm->mgmtData; // Get management data
    for (int i = 0; i < mgmt->numFrames; i++) {
        // If the frame is dirty and not currently pinned (fixCount == 0), write it to disk.
        if (mgmt->frames[i].dirty && mgmt->frames[i].fixCount == 0) {
            writeBlock(mgmt->frames[i].pageNum, &mgmt->fh, mgmt->frames[i].data); // Write the page to disk
            mgmt->frames[i].dirty = false; // Mark the page as clean
            mgmt->writeIO++; // Increment the writing I/O count
        }
    }
    return RC_OK; // Return success
}


/*
 * Pins a page into the buffer pool.
 *
 * @param bm The buffer pool.
 * @param page A pointer to the page handle to be updated.
 * @param pageNum The page number to pin.
 * @return RC_OK on success, or an error code if the page cannot be pinned.
 */
RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page,
           const PageNumber pageNum) {
    BM_MgmtData *mgmt = (BM_MgmtData *) bm->mgmtData;
    int frameIndex = findFrame(bm, pageNum);

    debugPrintAccess("Pinning", pageNum, mgmt->accessCounter);
    mgmt->accessCounter++;

    if (frameIndex != -1) {
        // Page is already in the buffer pool
        mgmt->frames[frameIndex].fixCount++;

        // Update access times based on strategy
        switch (bm->strategy) {
            case RS_FIFO:
                // FIFO doesn't update access times after the initial load
                break;
            case RS_LRU:
                mgmt->frames[frameIndex].lastTwo[1] = mgmt->accessCounter;
                break;
            case RS_LRU_K:
                mgmt->frames[frameIndex].lastTwo[0] = mgmt->frames[frameIndex].lastTwo[1];
                mgmt->frames[frameIndex].lastTwo[1] = mgmt->accessCounter;
                break;
            case RS_CLOCK:
                mgmt->frames[frameIndex].useBit = 1; // Set use bit when accessed
                break;
            case RS_LFU:
                mgmt->frames[frameIndex].accessCount++; // Increase access frequency
                break;
        }

        page->pageNum = pageNum;
        page->data = mgmt->frames[frameIndex].data;
        debugPrintPageHistory(bm, "After updating existing page");
        return RC_OK;
    }

    // Page not in pool, need replacement
    int victimIndex = selectVictim(bm);
    printf("Selected victim frame %d (currently contains page %d)\n",
           victimIndex, mgmt->frames[victimIndex].pageNum);

    if (victimIndex < 0 || victimIndex >= mgmt->numFrames) {
        return RC_NO_FREE_BUFFER_ERROR;
    }

    // Handle the victim page
    if (mgmt->frames[victimIndex].dirty) {
        RC rc = writeBlock(mgmt->frames[victimIndex].pageNum, &mgmt->fh,
                           mgmt->frames[victimIndex].data);
        if (rc != RC_OK) return rc;
        mgmt->writeIO++;
        mgmt->frames[victimIndex].dirty = false;
    }

    // Read the new page
    RC rc = readBlock(pageNum, &mgmt->fh, mgmt->frames[victimIndex].data);
    if (rc != RC_OK) {
        // If read fails, try to ensure capacity and read again
        rc = ensureCapacity(pageNum + 1, &mgmt->fh);
        if (rc != RC_OK) return rc;

        rc = readBlock(pageNum, &mgmt->fh, mgmt->frames[victimIndex].data);
        if (rc != RC_OK) return rc;
    }
    mgmt->readIO++;

    // Update frame metadata and access times based on strategy
    mgmt->frames[victimIndex].pageNum = pageNum;
    mgmt->frames[victimIndex].fixCount = 1;
    mgmt->frames[victimIndex].dirty = false;

    // Initialize access times for new page based on strategy
    switch (bm->strategy) {
        case RS_FIFO:
            mgmt->frames[victimIndex].lastTwo[0] = mgmt->accessCounter; // Initial load time
            mgmt->frames[victimIndex].lastTwo[1] = 0; // Not used in FIFO
            break;
        case RS_LRU:
            mgmt->frames[victimIndex].lastTwo[0] = 0; // Not used in LRU
            mgmt->frames[victimIndex].lastTwo[1] = mgmt->accessCounter; // Current access
            break;
        case RS_LRU_K:
            mgmt->frames[victimIndex].lastTwo[0] = 0; // No previous access
            mgmt->frames[victimIndex].lastTwo[1] = mgmt->accessCounter; // First access
            break;
        case RS_CLOCK:
        case RS_LFU:
            // Not implemented yet
            mgmt->frames[victimIndex].lastTwo[0] = 0;
            mgmt->frames[victimIndex].lastTwo[1] = 0;
            break;
        default:
            mgmt->frames[victimIndex].lastTwo[0] = 0;
            mgmt->frames[victimIndex].lastTwo[1] = 0;
            break;
    }

    page->pageNum = pageNum;
    page->data = mgmt->frames[victimIndex].data;

    return RC_OK;
}

/*
 * Marks a page as dirty in the buffer pool.  This indicates that the page has been modified
 * and needs to be written back to disk eventually.
 *
 * @param bm The buffer pool.
 * @param page The page handle representing the page to mark as dirty.
 * @return RC_OK on success, RC_PAGE_NOT_FOUND if the page is not in the buffer pool.
 */
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page) {
    BM_MgmtData *mgmtData = (BM_MgmtData *) bm->mgmtData; // Get the buffer pool's management data
    int frameIndex = findFrame(bm, page->pageNum); // Find the frame containing the page

    if (frameIndex == -1) return RC_PAGE_NOT_FOUND; // Return error if page not found

    mgmtData->frames[frameIndex].dirty = true; // Set the dirty flag for the frame
    return RC_OK; // Return success
}

/*
 * Unpins a page in the buffer pool.  This decrements the fix count of the page.
 * When the fix count reaches 0, the page can be selected as a victim for replacement.
 *
 * @param bm The buffer pool.
 * @param page The page handle representing the page to unpin.
 * @return RC_OK on success, RC_PAGE_NOT_FOUND if the page is not in the buffer pool.
 */
RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page) {
    BM_MgmtData *mgmtData = (BM_MgmtData *) bm->mgmtData; // Get the buffer pool's management data
    int frameIndex = findFrame(bm, page->pageNum); // Find the frame containing the page

    if (frameIndex == -1) return RC_PAGE_NOT_FOUND; // Return error if page not found

    // Only decrement fixCount if it is greater than 0.  This prevents over-unpinning.
    if (mgmtData->frames[frameIndex].fixCount > 0) {
        mgmtData->frames[frameIndex].fixCount--; // Decrement the fix count
    }

    return RC_OK; // Return success
}

/*
 * Forces a page to be written back to the disk immediately.  This ensures that the changes
 * made to the page are persistent.
 *
 * @param bm The buffer pool.
 * @param page The page handle representing the page to force.
 * @return RC_OK on success, RC_PAGE_NOT_FOUND if the page is not in the buffer pool,
 *         or RC_WRITE_FAILED if the write operation fails.
 */
RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page) {
    BM_MgmtData *mgmtData = (BM_MgmtData *) bm->mgmtData; // Get the buffer pool's management data
    int frameIndex = findFrame(bm, page->pageNum); // Find the frame containing the page

    if (frameIndex == -1) return RC_PAGE_NOT_FOUND; // Return error if page not found

    Frame *frame = &mgmtData->frames[frameIndex]; // Get a pointer to the frame

    if (frame->dirty) {
        // If the frame is dirty
        if (writeBlock(frame->pageNum, &mgmtData->fh, frame->data) != RC_OK) {
            // Write the page to disk
            return RC_WRITE_FAILED; // Return error if write fails
        }
        frame->dirty = false; // Mark the frame as clean
        mgmtData->writeIO++; // Increment the writing I/O count
    }
    return RC_OK; // Return success
}

/*
 * Gets an array containing the page numbers stored in each frame of the buffer pool.
 *
 * @param bm The buffer pool.
 * @return A dynamically allocated array of PageNumbers.  The caller is responsible for freeing this memory.
 *         Each element in the array corresponds to a frame in the buffer pool.
 *         If a frame is empty (contains no page), the corresponding element will be NO_PAGE.
 */
PageNumber *getFrameContents(BM_BufferPool *const bm) {
    BM_MgmtData *mgmtData = (BM_MgmtData *) bm->mgmtData; // Get the management data for the buffer pool
    PageNumber *contents = malloc(sizeof(PageNumber) * mgmtData->numFrames); // Allocate memory for the array

    // Iterate through each frame in the buffer pool
    for (int i = 0; i < mgmtData->numFrames; i++) {
        contents[i] = mgmtData->frames[i].pageNum; // Store the page number of the frame in the array
    }

    return contents; // Return the array of page numbers.  The Caller is responsible for freeing this.
}

/*
 * Gets an array indicating the dirty status of each frame in the buffer pool.
 *
 * @param bm The buffer pool.
 * @return A dynamically allocated array of booleans. The caller is responsible for freeing this memory.
 *         Each element in the array corresponds to a frame in the buffer pool.
 *         'true' indicates that the corresponding frame is dirty (modified), 'false' indicates it is clean.
 */
bool *getDirtyFlags(BM_BufferPool *const bm) {
    BM_MgmtData *mgmtData = (BM_MgmtData *) bm->mgmtData; // Get the management data
    bool *dirty = malloc(sizeof(bool) * mgmtData->numFrames); // Allocate memory for the boolean array

    // Iterate through each frame
    for (int i = 0; i < mgmtData->numFrames; i++) {
        dirty[i] = mgmtData->frames[i].dirty; // Store the dirty flag of the frame in the array
    }

    return dirty; // Return the array of dirty flags. The Caller is responsible for freeing this.
}

/*
 * Gets an array containing the fix counts for each frame in the buffer pool.
 * The fix count of a frame indicates how many times a page within that frame is currently pinned.
 *
 * @param bm The buffer pool.
 * @return A dynamically allocated array of integers.  The caller is responsible for freeing this memory.
 *         Each element in the array corresponds to a frame in the buffer pool.
 *         The value of each element is the fix count of the corresponding frame.
 */
int *getFixCounts(BM_BufferPool *const bm) {
    BM_MgmtData *mgmtData = (BM_MgmtData *) bm->mgmtData; // Get a pointer to the management data
    int *fixCounts = malloc(sizeof(int) * mgmtData->numFrames); // Allocate memory for the array of fix counts

    // Iterate through each frame in the buffer pool
    for (int i = 0; i < mgmtData->numFrames; i++) {
        fixCounts[i] = mgmtData->frames[i].fixCount; // Store the fix count of the current frame in the array
    }

    return fixCounts; // Return the array of fix counts. The Caller is responsible for freeing this memory.
}

/*
 * Gets the total number of read I/O operations performed on the buffer pool.
 * Each time a page is read from a disk into a frame, this counter is incremented.
 *
 * @param bm The buffer pool.
 * @return The total number of read I/O operations.
 */
int getNumReadIO(BM_BufferPool *const bm) {
    BM_MgmtData *mgmtData = (BM_MgmtData *) bm->mgmtData; // Get a pointer to the management data
    return mgmtData->readIO; // Return the number of read I/O operations
}

/*
 * Gets the total number of write I/O operations performed on the buffer pool.
 * Each time a dirty page is written back to disk, this counter is incremented.
 *
 * @param bm The buffer pool.
 * @return The total number of write I/O operations.
 */
int getNumWriteIO(BM_BufferPool *const bm) {
    BM_MgmtData *mgmtData = (BM_MgmtData *) bm->mgmtData; // Get a pointer to the management data
    return mgmtData->writeIO; // Return the number of write I/O operations
}

// Add this debug function at the top of the file
/*
 * Prints the current state of the buffer pool, including page numbers, fix counts, and access history for each frame.
 * This function is useful for debugging and understanding the behavior of the buffer manager.
 *
 * @param bm The buffer pool to print.
 * @param message A message to print before the buffer pool state.
 */
void debugPrintPageHistory(BM_BufferPool *const bm, const char *message) {
    BM_MgmtData *mgmt = (BM_MgmtData *) bm->mgmtData; // Get a pointer to the management data
    printf("\n=== %s ===\n", message); // Print the provided message
    printf("Current buffer state:\n");
    for (int i = 0; i < mgmt->numFrames; i++) {
        printf("Frame %d: Page %d, FixCount %d, History[%d, %d]\n", // Print frame details
               i,
               mgmt->frames[i].pageNum,
               mgmt->frames[i].fixCount,
               mgmt->frames[i].lastTwo[0],
               mgmt->frames[i].lastTwo[1]);
    }
    printf("========================\n");
}

// Also modify createDummyPages to ensure proper initialization:
/*
 * Creates a specified number of dummy pages and adds them to the buffer pool.
 * This function is used for testing purposes to populate the buffer pool with sample data.
 *
 * @param bm The buffer pool to add the dummy pages to.
 * @param num The number of dummy pages to create.
 */
void createDummyPages(BM_BufferPool *bm, int num) {
    int i;
    BM_PageHandle *h = MAKE_PAGE_HANDLE(); // Create a new page handle

    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL)); // Initialize the buffer pool

    // First ensure capacity.  This is important to prevent issues if the number of dummy pages exceeds the initial capacity.
    ensureCapacity(num, &((BM_MgmtData *) bm->mgmtData)->fh);
    // Ensure that the file has enough pages to hold the dummy pages.

    for (i = 0; i < num; i++) {
        CHECK(pinPage(bm, h, i)); // Pin the page with the specified page number.
        sprintf(h->data, "%s-%i", "Page", h->pageNum); // Format the page data with the page number.
        CHECK(markDirty(bm, h)); // Mark the page as dirty to simulate modifications.
        CHECK(unpinPage(bm,h)); // Unpin the page after writing the data.
    }

    CHECK(shutdownBufferPool(bm)); // Shutdown the buffer pool when done.
    free(h); // Free the page handle.
}

// Add this debug function
/*
 * Prints a message indicating an access to a page, including the type of access (pin/unpin) and the access counter value.
 * This function is used for debugging and tracking page access patterns.
 *
 * @param action The type of access ("Pin", "Unpin", etc.).
 * @param pageNum The page number that was accessed.
 * @param accessCounter The current value of the access counter.
 */
void debugPrintAccess(const char *action, PageNumber pageNum, int accessCounter) {
    printf("\n=== %s Page %d (Access time: %d) ===\n", action, pageNum, accessCounter);
}

// Add this to the top of the file with other strategy-specific functions
/*
 * Selects a victim frame using the Least Recently Used (LRU) replacement strategy.
 *
 * @param bm The buffer pool.
 * @return The index of the selected victim frame.
 */
static int selectLRUVictim(BM_BufferPool *const bm) {
    BM_MgmtData *mgmt = (BM_MgmtData *) bm->mgmtData; // Get a pointer to the management data
    int victim = -1; // Initialize the victim index.
    int oldestAccess = INT_MAX; // Initialize the oldest access time to the maximum possible integer value.

    for (int i = 0; i < mgmt->numFrames; i++) {
        if (mgmt->frames[i].fixCount == 0) {
            // Only consider frames that are not currently pinned.
            if (mgmt->frames[i].lastTwo[1] < oldestAccess) {
                // Check the last access time (lastTwo[1]) to find the least recently used page.
                oldestAccess = mgmt->frames[i].lastTwo[1]; // Update the oldest access time.
                victim = i; // Update the victim index.
            }
        }
    }
    return victim; // Return the index of the selected victim frame.
}
