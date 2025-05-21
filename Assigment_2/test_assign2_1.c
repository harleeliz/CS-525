/************************************************************
*     File name:               test_assign2_1.c             *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/

#include "storage_mgr.h" // Include the storage manager header for file operations
#include "buffer_mgr_stat.h" // Include the buffer manager statistics header for debugging
#include "buffer_mgr.h" // Include the buffer manager header for buffer pool operations
#include "dberror.h" // Include the error handling header
#include "test_helper.h" // Include the test helper header for assertions and test framework
#include <stdio.h> // Include the standard input/output library
#include <stdlib.h> // Include the standard library for memory allocation and other utilities
#include <string.h> // Include the string library for string manipulation

// Variable to store the current test's name
char *testName;

// Macro to check whether the content of a buffer pool is the same as an expected content
// (given in the format produced by sprintPoolContent)
#define ASSERT_EQUALS_POOL(expected,bm,message)           \
  do {          \
    char *real;        \
    char *_exp = (char *) (expected);                                   \
    real = sprintPoolContent(bm);     \
    if (strcmp((_exp),real) != 0)     \
      {         \
  printf("[%s-%s-L%i-%s] FAILED: expected <%s> but was <%s>: %s\n",TEST_INFO, _exp, real, message); \
  free(real);        \
  exit(1);       \
      }         \
    printf("[%s-%s-L%i-%s] OK: expected <%s> and was <%s>: %s\n",TEST_INFO, _exp, real, message); \
    free(real);        \
  } while(0)

// Test and helper methods declarations
static void testCreatingAndReadingDummyPages(void); // Test creating and reading fake pages
static void createDummyPages(BM_BufferPool *bm, int num); // Helper function to create fake pages
static void checkDummyPages(BM_BufferPool *bm, int num); // Helper function to check fake pages
static void testReadPage(void); // Test reading a single page
static void testFIFO(void); // Test FIFO page replacement strategy
static void testLRU(void); // Test LRU page replacement strategy

// Main method
int
main(void) {
    initStorageManager(); // Initialize the storage manager
    testName = ""; // Initialize the test name

    testCreatingAndReadingDummyPages(); // Run the test for creating and reading fake pages
    testReadPage(); // Run the test for reading a page
    testFIFO(); // Run the test for FIFO
    testLRU(); // Run the test for LRU

    return 0; // Return 0 to indicate successful execution
}

// Create n pages with content "Page X" and read them back to check whether the content is right
void
testCreatingAndReadingDummyPages(void) {
    BM_BufferPool *bm = MAKE_POOL(); // Create a new buffer pool
    testName = "Creating and Reading Back Dummy Pages"; // Set the test name

    CHECK(createPageFile("testbuffer.bin")); // Create a test page file

    createDummyPages(bm, 22); // Create 22 dummy pages
    checkDummyPages(bm, 20); // Check the first 20 fake pages

    createDummyPages(bm, 10000); // Create 10,000 dummy pages
    checkDummyPages(bm, 10000); // Check all 10,000 fake pages

    CHECK(destroyPageFile("testbuffer.bin")); // Destroy the test page file

    free(bm); // Free the buffer pool
    TEST_DONE(); // Mark the test as done
}


// Helper function to create fake pages
void
createDummyPages(BM_BufferPool *bm, int num) {
    int i;
    BM_PageHandle *h = MAKE_PAGE_HANDLE(); // Create a new page handle

    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));
    // Initialize the buffer pool with FIFO strategy and 3 frames

    for (i = 0; i < num; i++) {
        CHECK(pinPage(bm, h, i)); // Pin the page
        sprintf(h->data, "%s-%i", "Page", h->pageNum); // Format the page content
        CHECK(markDirty(bm, h)); // Mark the page as dirty
        CHECK(unpinPage(bm,h)); // Unpin the page
    }

    CHECK(shutdownBufferPool(bm)); // Shutdown the buffer pool

    free(h); // Free the page handle
}

// Helper function to check fake pages
void
checkDummyPages(BM_BufferPool *bm, int num) {
    int i;
    BM_PageHandle *h = MAKE_PAGE_HANDLE(); // Create a new page handle
    char *expected = malloc(sizeof(char) * 512); // Allocate memory for expected content

    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL)); // Initialize the buffer pool

    for (i = 0; i < num; i++) {
        CHECK(pinPage(bm, h, i)); // Pin the page

        sprintf(expected, "%s-%i", "Page", h->pageNum); // Format the expected content
        ASSERT_EQUALS_STRING(expected, h->data, "reading back dummy page content");
        // Assert that the actual content matches the expected content

        CHECK(unpinPage(bm,h)); // Unpin the page
    }

    CHECK(shutdownBufferPool(bm)); // Shutdown the buffer pool

    free(expected); // Free the expected content
    free(h); // Free the page handle
}

// Test reading a page
void
testReadPage() {
    BM_BufferPool *bm = MAKE_POOL(); // Create a new buffer pool
    BM_PageHandle *h = MAKE_PAGE_HANDLE(); // Create a new page handle
    testName = "Reading a page"; // Set the test name

    CHECK(createPageFile("testbuffer.bin")); // Create a test page file
    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL)); // Initialize the buffer pool

    CHECK(pinPage(bm, h, 0)); // Pin page 0
    CHECK(pinPage(bm, h, 0)); // Pin page 0 again (fix count should increment)

    CHECK(markDirty(bm, h)); // Mark the page as dirty

    CHECK(unpinPage(bm,h)); // Unpin page 0
    CHECK(unpinPage(bm,h)); // Unpin page 0

    CHECK(forcePage(bm, h)); // Force the page to disk

    CHECK(shutdownBufferPool(bm)); // Shutdown the buffer pool
    CHECK(destroyPageFile("testbuffer.bin")); // Destroy the test page file

    free(bm); // Free the buffer pool
    free(h); // Free the page handle

    TEST_DONE(); // Mark the test as done
}

// Test FIFO page replacement strategy
void
testFIFO() {
    // Expected results for buffer pool content at various stages of the test
    const char *poolContents[] = {
        "[0 0],[-1 0],[-1 0]",
        "[0 0],[1 0],[-1 0]",
        "[0 0],[1 0],[2 0]",
        "[3 0],[1 0],[2 0]",
        "[3 0],[4 0],[2 0]",
        "[3 0],[4 1],[2 0]",
        "[3 0],[4 1],[5x0]",
        "[6x0],[4 1],[5x0]",
        "[6x0],[4 1],[0x0]",
        "[6x0],[4 0],[0x0]",
        "[6 0],[4 0],[0 0]"
    };
    const int requests[] = {0, 1, 2, 3, 4, 4, 5, 6, 0}; // Sequence of page requests
    const int numLinRequests = 5; // Number of linear page requests
    const int numChangeRequests = 3; // Number of requests that cause page replacement

    int i;
    BM_BufferPool *bm = MAKE_POOL(); // Create a new buffer pool
    BM_PageHandle *h = MAKE_PAGE_HANDLE(); // Create a new page handle
    testName = "Testing FIFO page replacement"; // Set the test name

    CHECK(createPageFile("testbuffer.bin")); // Create a test page file

    createDummyPages(bm, 100); // Create 100 dummy pages

    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL)); // Initialize the buffer

    // reading some pages linearly with direct unpinning and no modifications
    for (i = 0; i < numLinRequests; i++) {
        pinPage(bm, h, requests[i]);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
    }

    // pin one page and test the remainder
    i = numLinRequests;
    pinPage(bm, h, requests[i]);
    ASSERT_EQUALS_POOL(poolContents[i], bm, "pool content after pin page");

    // read pages and mark them as dirty
    for (i = numLinRequests + 1; i < numLinRequests + numChangeRequests + 1; i++) {
        pinPage(bm, h, requests[i]);
        markDirty(bm, h);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[i], bm, "check pool content");
    }

    // flush buffer pool to disk
    i = numLinRequests + numChangeRequests + 1;
    h->pageNum = 4;
    unpinPage(bm, h);
    ASSERT_EQUALS_POOL(poolContents[i], bm, "unpin last page");

    i++;
    forceFlushPool(bm);
    ASSERT_EQUALS_POOL(poolContents[i], bm, "pool content after flush");

    //Check the number of writing I/Os
    ASSERT_EQUALS_INT(3, getNumWriteIO(bm), "check number of write I/Os");
    ASSERT_EQUALS_INT(8, getNumReadIO(bm), "check number of read I/Os");

    CHECK(shutdownBufferPool(bm));
    CHECK(destroyPageFile("testbuffer.bin"));

    free(bm);
    free(h);
    TEST_DONE();
}

// test the LRU page replacement strategy
void
testLRU(void) {
    // expected results
    const char *poolContents[] = {
        // Read the first five pages and directly unpin them
        "[0 0],[-1 0],[-1 0],[-1 0],[-1 0]",
        "[0 0],[1 0],[-1 0],[-1 0],[-1 0]",
        "[0 0],[1 0],[2 0],[-1 0],[-1 0]",
        "[0 0],[1 0],[2 0],[3 0],[-1 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        // use some of the page to create a fixed LRU order without changing pool content
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        // check that pages get evicted in LRU order
        "[0 0],[1 0],[2 0],[5 0],[4 0]",
        "[0 0],[1 0],[2 0],[5 0],[6 0]",
        "[7 0],[1 0],[2 0],[5 0],[6 0]",
        "[7 0],[1 0],[8 0],[5 0],[6 0]",
        "[7 0],[9 0],[8 0],[5 0],[6 0]"
    };
    const int orderRequests[] = {3, 4, 0, 2, 1};
    const int numLRUOrderChange = 5;

    int i;
    int snapshot = 0;
    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *h = MAKE_PAGE_HANDLE();
    testName = "Testing LRU page replacement";

    CHECK(createPageFile("testbuffer.bin"));
    createDummyPages(bm, 100);
    CHECK(initBufferPool(bm, "testbuffer.bin", 5, RS_LRU, NULL));

    // reading the first five pages linearly with direct unpinning and no modifications
    for (i = 0; i < 5; i++) {
        pinPage(bm, h, i);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot], bm, "check pool content reading in pages");
        snapshot++;
    }

    // read pages to change LRU order
    for (i = 0; i < numLRUOrderChange; i++) {
        pinPage(bm, h, orderRequests[i]);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot], bm, "check pool content using pages");
        snapshot++;
    }

    // replace pages and check that it happens in LRU order
    for (i = 0; i < 5; i++) {
        pinPage(bm, h, 5 + i);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot], bm, "check pool content using pages");
        snapshot++;
    }

    // Check the number of write I/Os
    ASSERT_EQUALS_INT(0, getNumWriteIO(bm), "check number of write I/Os");
    ASSERT_EQUALS_INT(10, getNumReadIO(bm), "check number of read I/Os");

    CHECK(shutdownBufferPool(bm));
    CHECK(destroyPageFile("testbuffer.bin"));

    free(bm);
    free(h);
    TEST_DONE();
}
