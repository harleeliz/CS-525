/************************************************************
*     File name:         test_assign1_1.c                   *
*     CS 525 Advanced Database Organization (Spring 2025)   *
*     Harlee Ramos , Jisun Yun, Baozhu Xie                  *
    ************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "storage_mgr.h"
#include "dberror.h"
#include "test_helper.h"

// Test name
char *testName;

// Test output files
#define TESTPF "test_pagefile.bin"

// Prototypes for test functions
static void testCreateOpenClose(void);

static void testSinglePageContent(void);

static void testMultiplePageContent(void);

static void testEnsureCapacity(void);

// Main function running all tests
int main(void) {
    testName = "";

    // Initialize the storage manager
    initStorageManager();

    // Run individual test cases
    testCreateOpenClose();
    testSinglePageContent();
    testMultiplePageContent();
    testEnsureCapacity();

    return 0;
}

// Test creating, opening, and closing a page file
void testCreateOpenClose(void) {
    SM_FileHandle fh;

    testName = "test create, open, and close methods";

    // Create a new page file
    TEST_CHECK(createPageFile(TESTPF));

    // Open the created page file
    TEST_CHECK(openPageFile(TESTPF, &fh));
    ASSERT_TRUE(strcmp(fh.fileName, TESTPF) == 0, "filename correct");
    ASSERT_TRUE((fh.totalNumPages == 1), "expect 1 page in new file");
    ASSERT_TRUE((fh.curPagePos == 0), "freshly opened file's page position should be 0");

    // Close the page file
    TEST_CHECK(closePageFile(&fh));

    // Destroy the page file
    TEST_CHECK(destroyPageFile(TESTPF));

    // Try opening a destroyed file (should return an error)
    ASSERT_TRUE((openPageFile(TESTPF, &fh) != RC_OK), "opening non-existing file should return an error.");

    TEST_DONE();
}

// Test writing and reading a single page
void testSinglePageContent(void) {
    SM_FileHandle fh;
    SM_PageHandle ph;
    int i;

    testName = "test single page content";

    // Allocate memory for the page
    ph = (SM_PageHandle) malloc(PAGE_SIZE);

    // Create a new page file
    TEST_CHECK(createPageFile(TESTPF));
    TEST_CHECK(openPageFile(TESTPF, &fh));
    printf("Created and opened file\n");

    // Read the first page into handle
    TEST_CHECK(readFirstBlock(&fh, ph));
    for (i = 0; i < PAGE_SIZE; i++) {
        ASSERT_TRUE((ph[i] == 0), "expected zero byte in first page of freshly initialized page");
    }
    printf("First block was empty\n");

    // Write a pattern to the first page
    for (i = 0; i < PAGE_SIZE; i++) {
        ph[i] = (i % 10) + '0';
    }
    TEST_CHECK(writeBlock(0, &fh, ph));
    printf("Writing first block\n");

    // Read back the first page and validate its content
    TEST_CHECK(readFirstBlock(&fh, ph));
    for (i = 0; i < PAGE_SIZE; i++) {
        ASSERT_TRUE((ph[i] == (i % 10) + '0'), "character in page read from disk is the one we expected.");
    }
    printf("Reading first block\n");

    // Destroy the page file
    TEST_CHECK(destroyPageFile(TESTPF));
    free(ph); // Free allocated memory

    TEST_DONE();
}

// Test multiple page content handling
void testMultiplePageContent(void) {
    SM_FileHandle fh;
    SM_PageHandle ph;
    int i;

    testName = "test multiple page content";

    // Allocate memory for the page
    ph = (SM_PageHandle) calloc(PAGE_SIZE, sizeof(char));

    // Create and open a new page file
    TEST_CHECK(createPageFile(TESTPF));
    TEST_CHECK(openPageFile(TESTPF, &fh));
    printf("Created and opened file\n");

    // Append a second page
    TEST_CHECK(appendEmptyBlock(&fh));
    ASSERT_TRUE((fh.totalNumPages == 2), "File should have 2 pages after appending an empty block.");

    // Write data to the second page
    for (i = 0; i < PAGE_SIZE; i++) {
        ph[i] = 'A';
    }
    TEST_CHECK(writeBlock(1, &fh, ph));
    printf("Writing second block\n");

    // Read back the second page and verify its content
    TEST_CHECK(readBlock(1, &fh, ph));
    for (i = 0; i < PAGE_SIZE; i++) {
        ASSERT_TRUE((ph[i] == 'A'), "character in second page read from disk is the one we expected.");
    }
    printf("Reading second block\n");

    // Close and destroy the file
    TEST_CHECK(closePageFile(&fh));
    TEST_CHECK(destroyPageFile(TESTPF));
    free(ph); // Free allocated memory

    TEST_DONE();
}

// Test ensuring capacity
void testEnsureCapacity(void) {
    SM_FileHandle fh;

    testName = "test ensure capacity";

    // Create and open a new page file
    TEST_CHECK(createPageFile(TESTPF));
    TEST_CHECK(openPageFile(TESTPF, &fh));

    // Ensure the file has at least 5 pages
    TEST_CHECK(ensureCapacity(5, &fh));
    ASSERT_TRUE((fh.totalNumPages == 5), "File should have 5 pages after ensuring capacity.");

    // Ensure the file has at least 10 pages
    TEST_CHECK(ensureCapacity(10, &fh));
    ASSERT_TRUE((fh.totalNumPages == 10), "File should have 10 pages after ensuring capacity.");

    // Close and destroy the file
    TEST_CHECK(closePageFile(&fh));
    TEST_CHECK(destroyPageFile(TESTPF));

    TEST_DONE();
}
