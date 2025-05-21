/************************************************************
 * File name:      record_mgr.c
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This file implements the Record Manager interface. It provides
 *   functionality for table creation, record operations (insert,
 *   update, delete, and get), scanning, and schema handling. It uses
 *   the Storage Manager and Buffer Manager to perform low-level I/O.
 ************************************************************/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // For access()

#include "tables.h"
#include "rm_serializer.h"
#include "record_mgr.h"
#include "buffer_mgr.h"
#include "storage_mgr.h"

/* ---------------------------------------------------------------------------
 * Data Structures
 * -------------------------------------------------------------------------*/

/*
 * Structure: ScanCondition
 * ------------------------
 * Holds the current state for a scan operation over a table.
 *
 * Members:
 *   currentPage - The current page being scanned.
 *   currentSlot - The current slot (record position) in the page.
 *   filter      - The expression filter to be applied during the scan.
 */
typedef struct ScanCondition {
    int currentPage;
    int currentSlot;
    Expr *filter;
} ScanCondition;

/* ---------------------------------------------------------------------------
 * Global Variables
 * -------------------------------------------------------------------------*/

// File handle for low-level page file operations
SM_FileHandle fileHandle;

// Buffer pool and page handle for page caching and management
BM_BufferPool *bufferPool;
BM_PageHandle *pageHandle;

// Head of the linked list for deserialized record nodes (used for record retrieval)
RecordNode *recordListHead = NULL;

// Global table metadata
int totalTuples = 0;         // Total number of tuples in the table
int recordSizeBytes;         // Size of a record in bytes (including any overhead)
int pageCapacity;            // Maximum number of records that can be stored in a page
int maxPageDirectories;      // Maximum number of page directory entries per page

/* ---------------------------------------------------------------------------
 * Initialization and Shutdown Functions
 * -------------------------------------------------------------------------*/

/*
 * Function: initRecordManager
 * ---------------------------
 * Initializes the Record Manager.
 * Currently, no extra initialization is performed.
 *
 * Parameters:
 *   mgmtData - A pointer to additional management data (unused).
 *
 * Returns:
 *   RC_OK on success.
 */
RC initRecordManager(void *mgmtData) {
    return RC_OK;
}

/*
 * Function: shutdownRecordManager
 * -------------------------------
 * Shuts down the Record Manager and releases any resources.
 *
 * Returns:
 *   RC_OK on success.
 */
RC shutdownRecordManager() {
    return RC_OK;
}

/* ---------------------------------------------------------------------------
 * Table Operations
 * -------------------------------------------------------------------------*/

/*
 * Function: createTable
 * ---------------------
 * Creates a new table by creating a page file, serializing the schema,
 * and initializing the page directory.
 *
 * Parameters:
 *   name   - The name of the table (and underlying file).
 *   schema - The schema that defines the table structure.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
RC createTable(char *name, Schema *schema) {
    if (name == NULL || schema == NULL)
        return RC_PARAMS_ERROR;

    // Check if the table already exists
    if (access(name, F_OK) == 0)
        return RC_TABLE_EXISTS;

    // Create the page file for the table
    if (createPageFile(name) != RC_OK)
        return RC_TABLE_CREATES_FAILED;

    // Open the file for writing
    if (openPageFile(name, &fileHandle) != RC_OK)
        return RC_ERROR;

    // Serialize the schema and write it to block 0
    char *schemaData = serializeSchema(schema);
    if (writeBlock(0, &fileHandle, schemaData) != RC_OK) {
        free(schemaData);
        return RC_WRITE_FAILED;
    }

    // Create the first page directory node (record data starts from page 2)
    PageDirectory *dirNode = createPageDirectoryNode(2);
    char *dirData = serializePageDirectory(dirNode);

    ensureCapacity(2, &fileHandle);
    if (writeBlock(1, &fileHandle, dirData) != RC_OK) {
        free(dirData);
        return RC_WRITE_FAILED;
    }

    // Close file to flush changes
    closePageFile(&fileHandle);

    // Initialize global metadata
    totalTuples = 0;
    recordSizeBytes = getRecordSize(schema) + sizeof(int) + sizeof(int) + 2 + 2 + 2 + 3 + 1 + 3 + 1;
    pageCapacity = 110;  // Reduced capacity to account for overhead
    maxPageDirectories = PAGE_SIZE / strlen(dirData);

    free(schemaData);
    free(dirNode);
    free(dirData);
    return RC_OK;
}

/*
 * Function: openTable
 * -------------------
 * Opens an existing table, loading its schema and page directory cache.
 *
 * Parameters:
 *   rel  - Pointer to an RM_TableData structure to be populated.
 *   name - The name of the table to open.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
RC openTable(RM_TableData *rel, char *name) {
    if (rel == NULL || name == NULL)
        return RC_PARAMS_ERROR;

    if (access(name, F_OK) == -1)
        return RC_TABLE_NOT_EXISTS;

    // Initialize buffer pool and page handle
    bufferPool = (BM_BufferPool *)malloc(sizeof(BM_BufferPool));
    pageHandle = (BM_PageHandle *)malloc(sizeof(BM_PageHandle));
    initBufferPool(bufferPool, name, 3, RS_FIFO, NULL);

    // Read schema from block 0
    pinPage(bufferPool, pageHandle, 0);
    Schema *tableSchema = deserializeSchema(pageHandle->data);
    unpinPage(bufferPool, pageHandle);

    // Read page directory cache from block 1
    pinPage(bufferPool, pageHandle, 1);
    PageDirectoryCache *dirCache = deserializePageDirectories(pageHandle->data);

    // Set table metadata
    rel->name = name;
    rel->schema = tableSchema;
    rel->mgmtData = dirCache;
    unpinPage(bufferPool, pageHandle);
    return RC_OK;
}

/*
 * Function: closeTable
 * --------------------
 * Closes the table, flushes changes, and frees allocated resources.
 *
 * Parameters:
 *   rel - Pointer to the RM_TableData structure representing the table.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
RC closeTable(RM_TableData *rel) {
    if (rel == NULL)
        return RC_PARAMS_ERROR;

    // Write updated page directory cache to block 1
    pinPage(bufferPool, pageHandle, 1);
    PageCache *cache = bufferPool->mgmtData;
    Frame *frame = searchPageFromCache(cache, pageHandle->pageNum);
    PageDirectoryCache *dirCache = rel->mgmtData;
    char *dirData = serializePageDirectories(dirCache);
    strcpy(frame->data, dirData);
    markDirty(bufferPool, pageHandle);
    unpinPage(bufferPool, pageHandle);
    forcePage(bufferPool, pageHandle);

    shutdownBufferPool(bufferPool);
    freeSchema(rel->schema);

    // Free the page directory linked list
    PageDirectory *curr = dirCache->front;
    while (curr != NULL) {
        PageDirectory *next = curr->next;
        free(curr);
        curr = next;
    }
    free(dirCache);
    free(pageHandle);
    return RC_OK;
}

/*
 * Function: deleteTable
 * ---------------------
 * Deletes the table by destroying its underlying page file.
 *
 * Parameters:
 *   name - The name of the table (and file) to delete.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
RC deleteTable(char *name) {
    if (name == NULL)
        return RC_FILE_NOT_FOUND;
    if (access(name, F_OK) == -1)
        return RC_TABLE_NOT_EXISTS;
    return destroyPageFile(name);
}

/*
 * Function: getNumTuples
 * ----------------------
 * Returns the number of tuples (records) currently in the table.
 *
 * Parameters:
 *   rel - Pointer to the RM_TableData structure.
 *
 * Returns:
 *   Total number of tuples.
 */
int getNumTuples(RM_TableData *rel) {
    return totalTuples;
}

/* ---------------------------------------------------------------------------
 * Page Utility Function
 * -------------------------------------------------------------------------*/

/*
 * Function: flushDataToPage
 * -------------------------
 * Writes a string of data into a specific page at a given offset.
 *
 * Parameters:
 *   data    - The data string to write.
 *   offset  - The offset within the page to start writing.
 *   pageNum - The page number where the data will be written.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
RC flushDataToPage(char *data, int offset, int pageNum) {
    if (data == NULL)
        return RC_PARAMS_ERROR;

    pinPage(bufferPool, pageHandle, pageNum);
    PageCache *cache = bufferPool->mgmtData;
    Frame *frame = searchPageFromCache(cache, pageHandle->pageNum);
    memset(frame->data + offset, '\0', strlen(data));
    strcpy(frame->data + offset, data);
    markDirty(bufferPool, pageHandle);
    unpinPage(bufferPool, pageHandle);
    forcePage(bufferPool, pageHandle);
    return RC_OK;
}

/* ---------------------------------------------------------------------------
 * Record Handling Functions
 * -------------------------------------------------------------------------*/

/*
 * Function: getRecords
 * --------------------
 * Deserializes record data from a page into a linked list of RecordNode
 * structures.
 *
 * Parameters:
 *   rel        - Pointer to the RM_TableData structure.
 *   recordData - The raw data from the page.
 *   size       - Size (in bytes) of each record.
 */
void getRecords(RM_TableData *rel, char *recordData, int size) {
    recordListHead = deserializeRecords(rel->schema, recordData, size);
}

/*
 * Function: insertRecord
 * ----------------------
 * Inserts a new record into the table. It finds an available slot by
 * checking the page directory cache and writes the serialized record data
 * to the appropriate page.
 *
 * Parameters:
 *   rel    - Pointer to the RM_TableData structure.
 *   record - Pointer to the Record to be inserted.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
RC insertRecord(RM_TableData *rel, Record *record) {
    if (rel == NULL || record == NULL)
        return RC_PARAMS_ERROR;

    Schema *schema = rel->schema;
    PageDirectoryCache *dirCache = rel->mgmtData;
    PageDirectory *targetDir = NULL;

    // Find a directory node with available space
    PageDirectory *curr = dirCache->front;
    while (curr != NULL) {
        if (curr->count < pageCapacity) {
            targetDir = curr;
            break;
        }
        curr = curr->next;
    }

    // If no free slot is found, create a new page directory node
    if (targetDir == NULL) {
        int newPageNum = dirCache->rear->pageNum + 1;
        PageDirectory *newDir = createPageDirectoryNode(newPageNum);
        if (dirCache->count % maxPageDirectories == 0) {
            newPageNum = dirCache->rear->pageNum + 1;
            char *tempDirData = serializePageDirectory(newDir);
            flushDataToPage(tempDirData, 0, newPageNum);
        }
        dirCache->rear->next = newDir;
        newDir->pre = dirCache->rear;
        dirCache->rear = newDir;
        dirCache->count++;
        targetDir = dirCache->rear;
    }

    if (targetDir == NULL)
        return RC_ERROR;

    int targetPage = targetDir->pageNum;
    int freeSlot = targetDir->firstFreeSlot;

    // Set record identifier and serialize record data
    record->id.page = targetPage;
    record->id.slot = freeSlot;
    int offset = record->id.slot * recordSizeBytes;
    char *serializedRecord = serializeRecord(record, schema);
    flushDataToPage(serializedRecord, offset, targetPage);

    // Update directory metadata and total tuple count
    targetDir->count++;
    targetDir->firstFreeSlot++;
    totalTuples++;
    return RC_OK;
}

/*
 * Function: deleteRecord
 * ----------------------
 * Deletes a record identified by its RID by marking it as deleted.
 *
 * Parameters:
 *   rel - Pointer to the RM_TableData structure.
 *   id  - The RID identifying the record to delete.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
RC deleteRecord(RM_TableData *rel, RID id) {
    if (rel == NULL)
        return RC_PARAMS_ERROR;

    PageDirectoryCache *dirCache = rel->mgmtData;
    PageDirectory *curr = dirCache->front;
    Schema *schema = rel->schema;

    while (curr != NULL) {
        if (curr->pageNum == id.page) {
            pinPage(bufferPool, pageHandle, curr->pageNum);

            // Create a temporary record to mark as deleted
            Record *tempRecord = (Record *)malloc(sizeof(Record));
            if (tempRecord == NULL)
                return RC_ALLOC_MEM_FAIL;
            tempRecord->id.page = -1;
            tempRecord->id.slot = -1;
            char *dataBuffer = (char*)calloc(recordSizeBytes, sizeof(char));
            tempRecord->data = dataBuffer;

            getRecord(rel, id, tempRecord);
            // Mark record as deleted by setting its id fields to 0
            tempRecord->id.page = 0;
            tempRecord->id.slot = 0;
            char *deletedData = serializeRecord(tempRecord, schema);
            int offset = recordSizeBytes * id.slot;

            // Write the "deleted" record into the page
            pinPage(bufferPool, pageHandle, curr->pageNum);
            PageCache *cache = bufferPool->mgmtData;
            Frame *frame = searchPageFromCache(cache, pageHandle->pageNum);
            strncpy(frame->data + offset, deletedData, recordSizeBytes);
            markDirty(bufferPool, pageHandle);
            unpinPage(bufferPool, pageHandle);
            forcePage(bufferPool, pageHandle);

            // Update record list and directory info
            getRecords(rel, pageHandle->data, recordSizeBytes);
            curr->count--;
            RecordNode *node = recordListHead;
            int idx = 0;
            while (node != NULL) {
                if (node->page == 0 && node->slot == 0) {
                    curr->firstFreeSlot = idx;
                    break;
                }
                idx++;
                node = node->next;
            }
            totalTuples--;
            break;
        }
        curr = curr->next;
    }
    return RC_OK;
}

/*
 * Function: updateRecord
 * ----------------------
 * Updates an existing record with new data.
 *
 * Parameters:
 *   rel    - Pointer to the RM_TableData structure.
 *   record - Pointer to the Record containing updated data.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
RC updateRecord(RM_TableData *rel, Record *record) {
    if (rel == NULL || record == NULL)
        return RC_PARAMS_ERROR;

    PageDirectoryCache *dirCache = rel->mgmtData;
    PageDirectory *curr = dirCache->front;
    Schema *schema = rel->schema;

    while (curr != NULL) {
        if (curr->pageNum == record->id.page) {
            pinPage(bufferPool, pageHandle, curr->pageNum);
            Record *tempRecord = (Record *)malloc(sizeof(Record));
            if (tempRecord == NULL)
                return RC_ALLOC_MEM_FAIL;
            tempRecord->id.page = -1;
            tempRecord->id.slot = -1;
            char *tempBuffer = (char*)calloc(recordSizeBytes, sizeof(char));
            tempRecord->data = tempBuffer;

            getRecord(rel, record->id, tempRecord);
            tempRecord->id.page = record->id.page;
            tempRecord->id.slot = record->id.slot;
            // Duplicate new record data
            tempRecord->data = strdup(record->data);
            char *updatedData = serializeRecord(tempRecord, schema);
            int offset = recordSizeBytes * tempRecord->id.slot;
            PageCache *cache = bufferPool->mgmtData;
            Frame *frame = searchPageFromCache(cache, curr->pageNum);
            strcpy(frame->data + offset, updatedData);
            markDirty(bufferPool, pageHandle);
            unpinPage(bufferPool, pageHandle);
            forcePage(bufferPool, pageHandle);
            free(tempRecord);
        }
        curr = curr->next;
    }
    return RC_OK;
}

/*
 * Function: getRecord
 * -------------------
 * Retrieves a record from the table given its RID.
 *
 * Parameters:
 *   rel    - Pointer to the RM_TableData structure.
 *   id     - The RID identifying the record.
 *   record - Pointer to a Record structure to populate with the record data.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
RC getRecord(RM_TableData *rel, RID id, Record *record) {
    if (rel == NULL || record == NULL)
        return RC_PARAMS_ERROR;

    record->id.page = id.page;
    record->id.slot = id.slot;
    Schema *schema = rel->schema;
    pinPage(bufferPool, pageHandle, id.page);
    getRecords(rel, pageHandle->data, recordSizeBytes);
    RecordNode *curr = recordListHead;
    while (curr != NULL) {
        if (id.page == curr->page && id.slot == curr->slot) {
            record->data = strdup(curr->data);
            break;
        }
        curr = curr->next;
    }
    if (record->data == NULL)
        return RC_ERROR;
    return RC_OK;
}

/* ---------------------------------------------------------------------------
 * Scan Operations
 * -------------------------------------------------------------------------*/

/*
 * Function: startScan
 * -------------------
 * Initializes a scan operation on a table based on a filter condition.
 *
 * Parameters:
 *   rel  - Pointer to the RM_TableData structure.
 *   scan - Pointer to the RM_ScanHandle to be initialized.
 *   cond - Filter condition (an expression) to apply during the scan.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond) {
    if (rel == NULL || scan == NULL || cond == NULL)
        return RC_PARAMS_ERROR;

    scan->mgmtData = malloc(sizeof(ScanCondition));
    if (scan->mgmtData == NULL)
        return RC_ALLOC_MEM_FAIL;

    ScanCondition *scanCond = (ScanCondition *)scan->mgmtData;
    scanCond->currentPage = 2;  // Data records start from page 2
    scanCond->currentSlot = 0;
    scanCond->filter = cond;

    scan->rel = rel;
    return RC_OK;
}

/*
 * Function: next
 * --------------
 * Retrieves the next record that satisfies the scan filter.
 *
 * Parameters:
 *   scan   - Pointer to the RM_ScanHandle.
 *   record - Pointer to a Record structure to store the retrieved record.
 *
 * Returns:
 *   RC_OK if a matching record is found, RC_RM_NO_MORE_TUPLES if the scan is complete,
 *   or an appropriate error code.
 */
RC next(RM_ScanHandle *scan, Record *record) {
    if (scan == NULL || record == NULL)
        return RC_ERROR;

    RM_TableData *rel = scan->rel;
    ScanCondition *scanCond = (ScanCondition *)scan->mgmtData;
    int currentPage = scanCond->currentPage;
    int currentSlot = scanCond->currentSlot;
    PageDirectoryCache *dirCache = rel->mgmtData;
    int maxPageNum = dirCache->rear->pageNum;

    // End scan if current page or slot exceeds limits
    if (currentPage > maxPageNum || (currentPage <= maxPageNum && currentSlot >= pageCapacity))
        return RC_RM_NO_MORE_TUPLES;

    while (scanCond->currentPage <= maxPageNum) {
        if (scanCond->currentSlot >= pageCapacity) {
            scanCond->currentSlot = 0;
            scanCond->currentPage++;
            if (scanCond->currentPage % (maxPageDirectories + 1) == 0)
                scanCond->currentPage++;
            continue;
        }
        RID rid;
        rid.page = scanCond->currentPage;
        rid.slot = scanCond->currentSlot;
        getRecord(rel, rid, record);
        scanCond->currentSlot++;
        if (scanCond->filter == NULL)
            return RC_OK;
        else {
            Value *result = malloc(sizeof(Value));
            evalExpr(record, rel->schema, scanCond->filter, &result);
            if (result != NULL && result->v.boolV != 0) {
                freeVal(result);
                return RC_OK;
            }
            freeVal(result);
        }
    }
    scanCond->currentSlot = -1;
    return RC_RM_NO_MORE_TUPLES;
}

/*
 * Function: closeScan
 * -------------------
 * Ends a scan operation and frees associated resources.
 *
 * Parameters:
 *   scan - Pointer to the RM_ScanHandle.
 *
 * Returns:
 *   RC_OK on success.
 */
RC closeScan(RM_ScanHandle *scan) {
    if (scan->mgmtData)
        free(scan->mgmtData);
    return RC_OK;
}

/* ---------------------------------------------------------------------------
 * Schema and Record Utilities
 * -------------------------------------------------------------------------*/

/*
 * Function: getRecordSize
 * -----------------------
 * Calculates the size (in bytes) required for a record based on its schema.
 *
 * Parameters:
 *   schema - Pointer to the Schema structure.
 *
 * Returns:
 *   The total size in bytes for a record.
 */
int getRecordSize(Schema *schema) {
    if (schema == NULL)
        return 0;

    int totalSize = 0;
    DataType *types = schema->dataTypes;
    int *lengths = schema->typeLength;
    for (int i = 0; i < schema->numAttr; i++) {
        if (types[i] == DT_INT)
            totalSize += sizeof(int);
        else if (types[i] == DT_STRING)
            totalSize += (sizeof(char) * lengths[i]);
        else if (types[i] == DT_BOOL)
            totalSize += sizeof(bool);
        else if (types[i] == DT_FLOAT)
            totalSize += sizeof(float);
    }
    return totalSize;
}

/*
 * Function: createSchema
 * ----------------------
 * Creates and initializes a new schema for a table.
 *
 * Parameters:
 *   numAttr   - Number of attributes.
 *   attrNames - Array of attribute names.
 *   dataTypes - Array of data types for each attribute.
 *   typeLength- Array of lengths for string attributes.
 *   keySize   - Number of key attributes.
 *   keys      - Array of indices of key attributes.
 *
 * Returns:
 *   A pointer to the newly created Schema.
 */
Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes,
                     int *typeLength, int keySize, int *keys) {
    if (attrNames == NULL || dataTypes == NULL || typeLength == NULL || keys == NULL)
        return NULL;

    Schema *schema = (Schema *)malloc(sizeof(Schema));
    if (schema == NULL)
        return NULL;

    schema->numAttr = numAttr;
    schema->attrNames = attrNames;
    schema->dataTypes = dataTypes;
    schema->typeLength = typeLength;
    schema->keyAttrs = keys;
    schema->keySize = keySize;
    return schema;
}

/*
 * Function: freeSchema
 * --------------------
 * Frees all memory allocated for a schema.
 *
 * Parameters:
 *   schema - Pointer to the Schema structure to be freed.
 *
 * Returns:
 *   RC_OK on success.
 */
RC freeSchema(Schema *schema) {
    if (schema == NULL)
        return RC_OK;

    int num = schema->numAttr;
    schema->numAttr = -1;
    if (schema->attrNames) {
        for (int i = 0; i < num; i++)
            free(schema->attrNames[i]);
        free(schema->attrNames);
        schema->attrNames = NULL;
    }
    if (schema->dataTypes) {
        free(schema->dataTypes);
        schema->dataTypes = NULL;
    }
    if (schema->typeLength) {
        free(schema->typeLength);
        schema->typeLength = NULL;
    }
    if (schema->keyAttrs) {
        free(schema->keyAttrs);
        schema->keyAttrs = NULL;
    }
    free(schema);
    schema = NULL;
    return RC_OK;
}

/*
 * Function: createRecord
 * ----------------------
 * Allocates and initializes a new record based on the provided schema.
 *
 * Parameters:
 *   record - Double pointer to the Record to be created.
 *   schema - Pointer to the Schema structure.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
RC createRecord(Record **record, Schema *schema) {
    if (record == NULL || schema == NULL)
        return RC_PARAMS_ERROR;

    Record *newRecord = (Record *)malloc(sizeof(Record));
    if (newRecord == NULL)
        return RC_ALLOC_MEM_FAIL;

    newRecord->id.page = -1;
    newRecord->id.slot = -1;
    int recSize = getRecordSize(schema);
    newRecord->data = (char *)calloc(recSize, sizeof(char));
    *record = newRecord;
    return RC_OK;
}

/*
 * Function: freeRecord
 * --------------------
 * Frees the memory associated with a record.
 *
 * Parameters:
 *   record - Pointer to the Record to be freed.
 *
 * Returns:
 *   RC_OK on success.
 */
RC freeRecord(Record *record) {
    if (record == NULL)
        return RC_OK;

    if (record->data) {
        free(record->data);
        record->data = NULL;
    }
    free(record);
    record = NULL;
    return RC_OK;
}

/* ---------------------------------------------------------------------------
 * Attribute Access Functions
 * -------------------------------------------------------------------------*/

/*
 * Function: getStringAttr
 * -----------------------
 * Retrieves a string attribute value from a record.
 *
 * Parameters:
 *   record    - Pointer to the Record.
 *   schema    - Pointer to the Schema.
 *   attrNum   - The attribute index.
 *   attrValue - Pointer to a Value structure where the string will be stored.
 *   offset    - The byte offset within the record data where the attribute starts.
 *
 * Returns:
 *   RC_OK on success.
 */
RC getStringAttr(Record *record, Schema *schema, int attrNum, Value *attrValue, int offset) {
    int strSize = sizeof(char) * schema->typeLength[attrNum];
    attrValue->v.stringV = (char *)malloc(strSize + 1);
    memcpy(attrValue->v.stringV, record->data + offset, strSize);
    attrValue->v.stringV[strSize] = '\0';
    return RC_OK;
}

/*
 * Function: getNumAttr
 * --------------------
 * Retrieves a numeric attribute (int, float, or bool) from a record.
 *
 * Parameters:
 *   record    - Pointer to the Record.
 *   schema    - Pointer to the Schema.
 *   attrNum   - The attribute index.
 *   attrValue - Pointer to a Value structure where the numeric value is stored.
 *   offset    - The byte offset within the record data where the attribute starts.
 *
 * Returns:
 *   RC_OK on success.
 */
RC getNumAttr(Record *record, Schema *schema, int attrNum, Value *attrValue, int offset) {
    int dataSize = 0;
    if (attrValue->dt == DT_INT)
        dataSize = sizeof(int);
    else if (attrValue->dt == DT_FLOAT)
        dataSize = sizeof(float);
    else if (attrValue->dt == DT_BOOL)
        dataSize = sizeof(bool);

    char *buffer = malloc(dataSize + 1);
    memcpy(buffer, record->data + offset, dataSize);
    buffer[dataSize] = '\0';
    // For simplicity, convert numeric value from string
    attrValue->v.intV = (int)strtol(buffer, NULL, 10);
    free(buffer);
    return RC_OK;
}

/*
 * Function: getAttr
 * -----------------
 * Retrieves the attribute value from a record and stores it in a Value pointer.
 *
 * Parameters:
 *   record - Pointer to the Record.
 *   schema - Pointer to the Schema.
 *   attrNum- The attribute index.
 *   value  - Double pointer to a Value where the attribute value will be stored.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
RC getAttr(Record *record, Schema *schema, int attrNum, Value **value) {
    if (record == NULL || schema == NULL || value == NULL)
        return RC_PARAMS_ERROR;

    int offset = 0;
    if (attrOffset(schema, attrNum, &offset) != RC_OK) {
        printf("Error calculating attribute offset\n");
        return RC_ERROR;
    }

    Value *attrVal = (Value *)malloc(sizeof(Value));
    DataType type = schema->dataTypes[attrNum];
    attrVal->dt = type;

    if (type == DT_STRING)
        getStringAttr(record, schema, attrNum, attrVal, offset);
    else if (type == DT_INT || type == DT_FLOAT || type == DT_BOOL)
        getNumAttr(record, schema, attrNum, attrVal, offset);
    else
        return RC_DATATYPE_UNDEFINE;

    *value = attrVal;
    return RC_OK;
}

/*
 * Function: intToString
 * ---------------------
 * Converts an integer to its string representation and stores it in a buffer.
 *
 * Parameters:
 *   numDigits - The number of digits to represent.
 *   value     - The integer value.
 *   buffer    - The output buffer to store the string.
 *
 * Notes:
 *   This function formats the integer into the buffer.
 */
void intToString(int numDigits, int value, char *buffer) {
    int remainder = 0;
    int temp = value;
    int pos = numDigits;
    while (temp > 0 && pos >= 0) {
        remainder = temp % 10;
        temp /= 10;
        buffer[pos] = buffer[pos] + remainder;
        pos--;
    }
    buffer[numDigits + 1] = '\0';
}

/*
 * Function: setAttr
 * -----------------
 * Sets an attribute's value in a record by writing the serialized value
 * into the record's data buffer at the appropriate offset.
 *
 * Parameters:
 *   record - Pointer to the Record.
 *   schema - Pointer to the Schema.
 *   attrNum- The attribute index.
 *   value  - Pointer to the Value to be set.
 *
 * Returns:
 *   RC_OK on success or an appropriate error code.
 */
RC setAttr(Record *record, Schema *schema, int attrNum, Value *value) {
    if (record == NULL || schema == NULL || value == NULL)
        return RC_PARAMS_ERROR;

    int offset = 0;
    if (attrOffset(schema, attrNum, &offset) != RC_OK) {
        printf("Error calculating attribute offset\n");
        return RC_ERROR;
    }

    if (value->dt != schema->dataTypes[attrNum])
        return RC_DATATYPE_MISMATCH;

    if (value->dt == DT_STRING)
        sprintf(record->data + offset, "%s", value->v.stringV);
    else if (value->dt == DT_INT) {
        char dataBuffer[5];  // Assuming 3-digit numbers plus null terminator
        intToString(3, value->v.intV, dataBuffer);
        sprintf(record->data + offset, "%s", dataBuffer);
    }
    else if (value->dt == DT_FLOAT)
        sprintf(record->data + offset, "%f", value->v.floatV);
    else if (value->dt == DT_BOOL)
        sprintf(record->data + offset, "%d", value->v.boolV);
    else
        return RC_ERROR;

    return RC_OK;
}