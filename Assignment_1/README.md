# Assigment N°1. Storage Manager Implementation
### CS 525 Advanced Database Organization (Spring 2025)
Professor: Gerald Balekaki <br>
TAs: Mohammadreza Sediqin, Nency Patel, Rashmi Topannavar, Ramani Chinthakindi   <br>
Students: Harlee Ramos (A20528450) , Jisun Yun (A20536679), Baozhu Xie () <br>

**1. Introduction**
This project implements a storage manager capable of managing fixed-size (4KB) pages in database files. Key features include:
* File lifecycle management: Create, open, close, and destroy page files
* Page I/O operations: Read/write pages using absolute or relative addressing
* Metadata tracking: Maintain file size, current position, and management info
* Capacity control: Expand files with zero-initialized pages as needed
***
**2. Code Structure**
The code is structured as follows:

| File Name          | Description                                         |
|-------------------|-----------------------------------------------------|
| dberror.c         | Defines error-handling functions.                  |
| dberror.h         | Contains error codes and macros.                     |
| storage_mgr.c      | Implements the core storage manager functions.     |
| storage_mgr.h      | Declares the storage manager interface.             |
| test_assign1_1.c   | Contains test cases for the storage manager.         |
| test_helper.h      | Provides macros and utilities for testing.          |
| Makefile           | Builds the project and compiles the tests.          |
| README.md         | Explains the structure, functionality, and testing instructions of the project.  |
***
**3. Storage Manager Functions**
The storage manager provides functionalities for initialization, file operations, page read/write operations, and capacity management.

**3.1. Initialization**
* `initStorageManager()`: Initializes the storage manager (prints a confirmation message).
Code section:
```
  void initStorageManager(void) {
  printf("Storage Manager Initialized.\n");}
```

**3.2. File Operations**
* Function: `createPageFile(char *fileName)`: Creates a new file with a single page initialized to zero bytes.
  Code section:
```
  RC createPageFile(char *fileName) {
    
    FILE *fp = fopen(fileName, "w+");
   
    if (fp == NULL) {
        return RC_FILE_NOT_FOUND;}
    
    char *emptyPage = (char *)calloc(PAGE_SIZE, sizeof(char));
   
    if (emptyPage == NULL) {
        fclose(fp);
        return RC_WRITE_FAILED;}
    
    size_t bytesWritten = fwrite(emptyPage, sizeof(char), PAGE_SIZE, fp);
    free(emptyPage);
    fclose(fp);

    if (bytesWritten != PAGE_SIZE) {
        return RC_WRITE_FAILED; }
    return RC_OK;
}
```
* Function: `openPageFile(char *fileName, SM_FileHandle *fHandle)`: Opens an existing file and initializes metadata in the provided `SM_FileHandle` struct.
  Code section:
```
  RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    FILE *fp = fopen(fileName, "r+");
   
   if (fp == NULL) {
        return RC_FILE_NOT_FOUND;}
    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    
    if (fileSize % PAGE_SIZE != 0) {
        fclose(fp);
        return RC_READ_NON_EXISTING_PAGE; }
    
    fHandle->fileName = fileName;
    fHandle->totalNumPages = fileSize / PAGE_SIZE;
    fHandle->curPagePos = 0;
    fHandle->mgmtInfo = fp;

    return RC_OK;}
```
* Function: `closePageFile(SM_FileHandle *fHandle)`: Closes the opened file and clears the `mgmtInfo` pointer in the handle. 
Code section:
```
  RC closePageFile(SM_FileHandle *fHandle) {
    if (fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;}
        
    fclose((FILE *)fHandle->mgmtInfo);
    fHandle->mgmtInfo = NULL;
    return RC_OK;}

```
* Function: `destroyPageFile(char *fileName)`: Deletes the specified file from the system.
  Code section:
```
  RC destroyPageFile(char *fileName) {
    if (remove(fileName) != 0) {
        return RC_FILE_NOT_FOUND;}
    return RC_OK;}
```

**3.3. Page Read/Write Operations**
* `readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)`: Reads a specific page from the file into memory. Returns an error if the page doesn't exist.
  Code section:
```
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;}
        
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        return RC_READ_NON_EXISTING_PAGE;}
    
    FILE *fp = (FILE *)fHandle->mgmtInfo;
    
    if (fseek(fp, pageNum * PAGE_SIZE, SEEK_SET) != 0) {
        return RC_READ_NON_EXISTING_PAGE; }
    
    size_t bytesRead = fread(memPage, sizeof(char), PAGE_SIZE, fp);
   
    if (bytesRead != PAGE_SIZE) {
        return RC_READ_NON_EXISTING_PAGE;}
    
    fHandle->curPagePos = pageNum;
    return RC_OK;}
```
* `writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage)`: Writes data from memory to a specific page in the file. Returns an error if the page doesn't exist.
  Code section:
```
  RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
   
    if (fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT; }
    
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        return RC_WRITE_FAILED;}
   
    FILE *fp = (FILE *)fHandle->mgmtInfo;
    
    if (fseek(fp, pageNum * PAGE_SIZE, SEEK_SET) != 0) {
        return RC_WRITE_FAILED; }
    
    size_t bytesWritten = fwrite(memPage, sizeof(char), PAGE_SIZE, fp);
    
    if (bytesWritten != PAGE_SIZE) {
        return RC_WRITE_FAILED;}
    
    fHandle->curPagePos = pageNum;
    return RC_OK;}
```
* Note: Additional helper functions provide relative addressing for reading pages from the file:
	* `readFirstBlock`, `readLastBlock`, `readCurrentBlock`, `readNextBlock`, `readPreviousBlock`.

**3.4. Capacity Management**
* Function: `appendEmptyBlock(SM_FileHandle *fHandle)`: Appends a new empty page (initialized to zero bytes) to the file.
  Code section:
```
RC appendEmptyBlock(SM_FileHandle *fHandle) {
    if (fHandle->mgmtInfo == NULL) {
        return RC_FILE_HANDLE_NOT_INIT;}
        
    FILE *fp = (FILE *)fHandle->mgmtInfo;
    char *emptyPage = (char *)calloc(PAGE_SIZE, sizeof(char));
    
    if (emptyPage == NULL) {
        return RC_WRITE_FAILED;}
        
    if (fseek(fp, 0, SEEK_END) != 0) {
        free(emptyPage);
        return RC_WRITE_FAILED; }
   
    size_t bytesWritten = fwrite(emptyPage, sizeof(char), PAGE_SIZE, fp);
    free(emptyPage);

    if (bytesWritten != PAGE_SIZE) {
        return RC_WRITE_FAILED; }
   
    fHandle->totalNumPages++;
    return RC_OK;}
```

* Function: `ensureCapacity(int numberOfPages, SM_FileHandle *fHandle)`: Ensures the file has at least the specified number of pages by appending empty pages as needed.
  
```
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle) {
  if (numberOfPages < 0) {
  return RC_OK; // Invalid input, treat as success or return error}

  while (fHandle->totalNumPages < numberOfPages) {
  RC rc = appendEmptyBlock(fHandle);
  
  if (rc != RC_OK) {return rc;}}
  return RC_OK;}
```
***
**4. Test Cases**
The `test_assign1_1.c` file implements unit tests to validate the storage manager's functionalities. Some key test cases include: 
<br>
* 4.1.	testCreateOpenClose():Verifies creating, opening, and closing a page file.
Code section:
```
void testCreateOpenClose(void) {
SM_FileHandle fh;
TEST_CHECK(createPageFile(TESTPF));
TEST_CHECK(openPageFile(TESTPF, &fh));
ASSERT_TRUE(strcmp(fh.fileName, TESTPF) == 0, "filename correct");
TEST_CHECK(closePageFile(&fh));
TEST_CHECK(destroyPageFile(TESTPF));}
```   

* 4.2.	testSinglePageContent():Tests writing and reading content to/from a single page.
Code section:
```
void testSinglePageContent(void) {
    SM_FileHandle fh;
    SM_PageHandle ph = (SM_PageHandle)malloc(PAGE_SIZE);
    TEST_CHECK(createPageFile(TESTPF));
    TEST_CHECK(openPageFile(TESTPF, &fh));
    TEST_CHECK(readFirstBlock(&fh, ph));
    TEST_CHECK(writeBlock(0, &fh, ph));
    free(ph);}
```
* 4.3.	testMultiplePageContent():Tests handling multiple pages, including appending and reading them.
Code section:
```
void testMultiplePageContent(void) {
    SM_FileHandle fh;
    SM_PageHandle ph = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
    TEST_CHECK(createPageFile(TESTPF));
    TEST_CHECK(openPageFile(TESTPF, &fh));
    TEST_CHECK(appendEmptyBlock(&fh));
    free(ph);}
```

* 4.4.	testEnsureCapacity(): Ensures the file grows to the required capacity.
Code section:
```
void testEnsureCapacity(void) {
SM_FileHandle fh;
TEST_CHECK(createPageFile(TESTPF));
TEST_CHECK(openPageFile(TESTPF, &fh));
TEST_CHECK(ensureCapacity(10, &fh));}
```
Debugging Aids: `printError()` → Prints error messages
`CHECK macro` → Validates return codes
***
**5. Build and Execution Instructions**
* Navigate to the project directory: `cd /path/to/project`
* Build the project: `make all`
* Execute the tests: `./test_assign1`
* Debug and re-run as needed: `make clean`
* See pdf file for example in Terminal and CLion
***
**6. Error Handling:**
* The storage manager uses return codes `RC` to indicate the success or failure of operations. The `dberror.c` module manages error handling by:
  * Defining error codes and associated messages.
  * Providing functions to retrieve and print error messages. 
  * Using a global variable `RC_message` to store additional error information.
***
**7. Memory Management:**
The code ensures that all dynamically allocated memory, such as memory for pages, is properly freed using the `free()` function when it is no longer needed.


