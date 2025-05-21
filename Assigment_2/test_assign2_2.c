/************************************************************
*     File name:               test_assign2_2.c             *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
 ************************************************************/

// @formatter: wrap_before_expression_rbrace false
// @formatter: wrap_after_expression_lbrace false
// @formatter: remove_blank_lines_near_braces_in_code false
#include "storage_mgr.h"
#include "buffer_mgr_stat.h"
#include "buffer_mgr.h"
#include "dberror.h"
#include "test_helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// var to store the current test's name
char *testName;

// check whether two the content of a buffer pool is the same as an expected content
// (given in the format produced by sprintPoolContent)
#define ASSERT_EQUALS_POOL(expected,bm,message)                    \
do {                                    \
char *real;                                \
char *_exp = (char *) (expected);                                   \
real = sprintPoolContent(bm);                    \
if (strcmp((_exp),real) != 0)                    \
{                                    \
printf("[%s-%s-L%i-%s] FAILED: expected <%s> but was <%s>: %s\n",TEST_INFO, _exp, real, message); \
free(real);                            \
exit(1);                            \
}                                    \
printf("[%s-%s-L%i-%s] OK: expected <%s> and was <%s>: %s\n",TEST_INFO, _exp, real, message); \
free(real);                                \
} while(0)

// test and helper methods
static void testLRU_K(void);

static void testError(void);

// main method
int
main(void) {
    initStorageManager();
    testName = "";

    testLRU_K();
    testError();
    return 0;
}

void debugPrintFixCounts(BM_BufferPool *bm);

// test the LRU_K page replacement strategy
void testLRU_K(void) {
    // expected results
    const char *poolContents[] = {
        "[0 0],[-1 0],[-1 0],[-1 0],[-1 0]",
        "[0 0],[1 0],[-1 0],[-1 0],[-1 0]",
        "[0 0],[1 0],[2 0],[-1 0],[-1 0]",
        "[0 0],[1 0],[2 0],[3 0],[-1 0]",
        "[0 0],[1 0],[2 0],[3 0],[4 0]",
        "[0 0],[1 0],[2 0],[5 0],[4 0]",
        "[0 0],[1 0],[2 0],[5 0],[6 0]",
        "[7 0],[1 0],[2 0],[5 0],[6 0]",
        "[7 0],[1 0],[8 0],[5 0],[6 0]",
        "[7 0],[9 0],[8 0],[5 0],[6 0]"
    };

    const int orderRequests[] = {3, 4, 0, 2, 1};
    const int numLRU_KOrderChange = 5;

    int i;
    int snapshot = 0;
    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *h = MAKE_PAGE_HANDLE();
    testName = "Testing LRU_K page replacement";

    // Create and initialize the page file first
    CHECK(createPageFile("testbuffer.bin"));

    // Initialize buffer pool with K=2
    int k = 2;
    void *stratData = &k;
    CHECK(initBufferPool(bm, "testbuffer.bin", 5, RS_LRU_K, stratData));

    // Create dummy pages after buffer pool is initialized
    for (i = 0; i < 100; i++) {
        CHECK(pinPage(bm, h, i));
        sprintf(h->data, "%s-%i", "Page", h->pageNum);
        CHECK(markDirty(bm, h));
        CHECK(unpinPage(bm, h));
    }

    // Shutdown and reinitialize for the actual test
    CHECK(shutdownBufferPool(bm));
    CHECK(initBufferPool(bm, "testbuffer.bin", 5, RS_LRU_K, stratData));

    // First phase: Read pages 0-4
    for (i = 0; i < 5; i++) {
        pinPage(bm, h, i);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content reading in pages");
    }

    // Second phase: Create history by accessing pages multiple times
    for (i = 0; i < numLRU_KOrderChange; i++) {
        pinPage(bm, h, orderRequests[i]);
        unpinPage(bm, h);
        // Add a second access to create history
        if (i < 3) {
            pinPage(bm, h, orderRequests[i]);
            unpinPage(bm, h);
        }
    }

    // Third phase: Replace pages and check that it happens in LRU-K order
    for (i = 5; i < 10; i++) {
        pinPage(bm, h, i);
        unpinPage(bm, h);
        ASSERT_EQUALS_POOL(poolContents[snapshot++], bm, "check pool content using pages");
    }

    // Check the number of write I/Os
    ASSERT_EQUALS_INT(0, getNumWriteIO(bm), "check number of write I/Os");
    ASSERT_EQUALS_INT(10, getNumReadIO(bm), "check number of read I/Os");

    // Ensure all pages are unpinned before shutdown
    for (i = 0; i < bm->numPages; i++) {
        h->pageNum = i;
        int frameIndex = findFrame(bm, i);
        if (frameIndex != -1 && getFixCounts(bm)[frameIndex] > 0) {
            unpinPage(bm, h);
        }
    }

    // Print final fix counts
    debugPrintFixCounts(bm);

    // Shutdown buffer pool once
    CHECK(shutdownBufferPool(bm));

    // Clean up
    CHECK(destroyPageFile("testbuffer.bin"));
    free(bm);
    free(h);
    TEST_DONE();
}


void debugPrintFixCounts(BM_BufferPool *bm) {
    int *fixCounts = getFixCounts(bm);
    printf("Fix counts before shutdown: [");
    for (int i = 0; i < bm->numPages; i++) {
        printf("%d ", fixCounts[i]);
    }
    printf("]\n");
    free(fixCounts);
}


// test error cases
void
testError(void) {
    BM_BufferPool *bm = MAKE_POOL();
    BM_PageHandle *h = MAKE_PAGE_HANDLE();
    testName = "ERROR TEST";

    CHECK(createPageFile("testbuffer.bin"));

    // pinpage until the buffer pool is full and then request additional page.
    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));
    CHECK(pinPage(bm, h, 0));
    CHECK(pinPage(bm, h, 1));
    CHECK(pinPage(bm, h, 2));

    ASSERT_ERROR(initBufferPool(bm, "unavailable.bin", 3, RS_FIFO, NULL),
                 "try to init buffer pool for non existing page file");

    CHECK(unpinPage(bm, h)); // Unpin page 2 (last pinned)
    CHECK(unpinPage(bm, h)); // Unpin page 1
    CHECK(unpinPage(bm, h)); // Unpin page 0 (first pinned)

    CHECK(shutdownBufferPool(bm));

    // try to pin page with negative page number.
    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));
    ASSERT_ERROR(pinPage(bm, h, -10), "try to pin page with negative page number");
    CHECK(shutdownBufferPool(bm));


    // try to use uninitialized buffer pool
    ASSERT_ERROR(initBufferPool(bm, "unavailable.bin", 3, RS_FIFO, NULL),
                 "try to init buffer pool for non existing page file");
    ASSERT_ERROR(shutdownBufferPool(bm), "shutdown buffer pool that is not open");
    ASSERT_ERROR(forceFlushPool(bm), "flush buffer pool that is not open");
    ASSERT_ERROR(pinPage(bm, h, 1), "pin page in buffer pool that is not open");


    // try to unpin, mark, or force page that is not in pool
    CHECK(initBufferPool(bm, "testbuffer.bin", 3, RS_FIFO, NULL));
    ASSERT_ERROR(unpinPage(bm, h), "Try to unpin a page which is not available in framelist.");
    ASSERT_ERROR(forcePage(bm, h), "Try to forceflush a page which is not available in framelist.");
    ASSERT_ERROR(markDirty(bm, h), "Try to markdirty a page which is not available in framelist.");
    CHECK(shutdownBufferPool(bm));

    // Done remove a page file
    CHECK(destroyPageFile("testbuffer.bin"));

    free(bm);
    free(h);
    TEST_DONE();
}

// @formatter: remove_blank_lines_near_braces_in_code restore
// @formatter: wrap_after_expression_lbrace restore
// @formatter: wrap_before_expression_rbrace restore
