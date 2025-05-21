/************************************************************
 * File name:      record_mgr.h
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This header file declares the Record Manager interface, which
 *   provides functions for managing tables, records, and scans.
 *   It includes functions for initializing/shutting down the
 *   record manager, handling table and record operations, performing
 *   scans on tables, managing schemas, and handling record attributes.
 *   Additionally, a helper function is declared to create page directory
 *   nodes (used for internal bookkeeping).
 ************************************************************/

#ifndef RECORD_MGR_H
#define RECORD_MGR_H

#include "dberror.h"          // Error codes and logging
#include "storage_mgr.h"      // Low-level storage management functions
#include "buffer_mgr_stat.h"  // Buffer manager statistics functions
#include "buffer_mgr.h"       // Buffer manager interface
#include "expr.h"             // Expression evaluation for scan conditions
#include "tables.h"           // Table schema and metadata definitions
#include "test_helper.h"      // Helper functions for testing
#include "rm_serializer.h"    // Serialization/deserialization of records

/*
 * Structure: RM_ScanHandle
 * ------------------------
 * Contains the necessary information to perform a scan over a table.
 *
 * Members:
 *   rel      - Pointer to the table (of type RM_TableData) to be scanned.
 *   mgmtData - Management data used to keep track of the scan state.
 */
typedef struct RM_ScanHandle {
	/* Reference to the table being scanned */
	RM_TableData *rel;
	/* Additional scan management data (e.g., current position) */
	void *mgmtData;
} RM_ScanHandle;

/* ===========================================================
 *                    Record Manager Functions
 * ===========================================================
 *
 * The following functions comprise the Record Manager API:
 *
 * 1. Initialization and Shutdown:
 *    - initRecordManager: Set up the record manager with any necessary
 *      management data.
 *    - shutdownRecordManager: Clean up and release resources used by the
 *      record manager.
 *
 * 2. Table Operations:
 *    - createTable: Create a new table with a given name and schema.
 *    - openTable: Open an existing table and load its metadata.
 *    - closeTable: Close an open table.
 *    - deleteTable: Delete a table from disk.
 *    - getNumTuples: Return the number of records (tuples) in a table.
 *
 * 3. Record Operations:
 *    - insertRecord: Insert a new record into a table.
 *    - deleteRecord: Delete an existing record using its Record ID.
 *    - updateRecord: Update an existing record.
 *    - getRecord: Retrieve a record by its Record ID.
 *
 * 4. Scan Functions:
 *    - startScan: Begin a scan on a table with an optional condition.
 *    - next: Retrieve the next record that matches the scan condition.
 *    - closeScan: End the scan and release any associated resources.
 *
 * 5. Schema Management:
 *    - getRecordSize: Calculate the size of a record based on its schema.
 *    - createSchema: Create a schema for a table, specifying attributes,
 *      data types, and keys.
 *    - freeSchema: Free the memory allocated for a schema.
 *
 * 6. Record and Attribute Handling:
 *    - createRecord: Allocate and initialize a new record.
 *    - freeRecord: Free the memory allocated for a record.
 *    - getAttr: Retrieve an attribute value from a record.
 *    - setAttr: Set an attribute value in a record.
 *
 * 7. Helper Function:
 *    - createPageDirectoryNode: Create a new node for managing page directories.
 */

/* -------------------------
 * Initialization and Shutdown
 * -------------------------
 */

/* Initializes the Record Manager.
 * mgmtData can be used to pass custom initialization data if needed.
 */
extern RC initRecordManager(void *mgmtData);

/* Shuts down the Record Manager and releases any allocated resources. */
extern RC shutdownRecordManager(void);

/* -------------------------
 * Table Operations
 * -------------------------
 */

/* Creates a new table with the specified name and schema. */
extern RC createTable(char *name, Schema *schema);

/* Opens an existing table and loads its metadata into rel. */
extern RC openTable(RM_TableData *rel, char *name);

/* Closes an open table, ensuring any buffered changes are flushed. */
extern RC closeTable(RM_TableData *rel);

/* Deletes the table with the specified name from disk. */
extern RC deleteTable(char *name);

/* Returns the number of tuples (records) in the table. */
extern int getNumTuples(RM_TableData *rel);

/* -------------------------
 * Record Operations
 * -------------------------
 */

/* Inserts a new record into the specified table. */
extern RC insertRecord(RM_TableData *rel, Record *record);

/* Deletes the record identified by id from the table. */
extern RC deleteRecord(RM_TableData *rel, RID id);

/* Updates an existing record in the table. */
extern RC updateRecord(RM_TableData *rel, Record *record);

/* Retrieves a record identified by id from the table. */
extern RC getRecord(RM_TableData *rel, RID id, Record *record);

/* -------------------------
 * Scan Functions
 * -------------------------
 */

/* Begins a scan on the table with an optional condition.
 * The scan handle will be initialized for iterating over records.
 */
extern RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond);

/* Retrieves the next record that satisfies the scan condition. */
extern RC next(RM_ScanHandle *scan, Record *record);

/* Closes the scan and frees any resources associated with it. */
extern RC closeScan(RM_ScanHandle *scan);

/* -------------------------
 * Schema Management
 * -------------------------
 */

/* Calculates the size (in bytes) of a record based on the schema. */
extern int getRecordSize(Schema *schema);

/* Creates a new schema for a table with the specified attributes and keys.
 * numAttr: Number of attributes.
 * attrNames: Array of attribute names.
 * dataTypes: Array of data types for the attributes.
 * typeLength: Array of lengths for each attribute type.
 * keySize: Number of attributes that form the key.
 * keys: Array of key attribute indices.
 */
extern Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes,
                            int *typeLength, int keySize, int *keys);

/* Frees the memory allocated for a schema. */
extern RC freeSchema(Schema *schema);

/* -------------------------
 * Record and Attribute Handling
 * -------------------------
 */

/* Creates a new record associated with the given schema. */
extern RC createRecord(Record **record, Schema *schema);

/* Frees the memory allocated for a record. */
extern RC freeRecord(Record *record);

/* Retrieves the attribute value from a record at the given attribute number. */
extern RC getAttr(Record *record, Schema *schema, int attrNum, Value **value);

/* Sets the attribute value in a record at the given attribute number. */
extern RC setAttr(Record *record, Schema *schema, int attrNum, Value *value);

/* -------------------------
 * Helper Functions
 * -------------------------
 */

/*
 * Helper Function: createPageDirectoryNode
 * ------------------------------------------
 * Creates a new node for a page directory given a page number.
 *
 * Parameters:
 *   pageNum - The page number for which to create a directory node.
 *
 * Returns:
 *   A pointer to the newly created PageDirectory node.
 *
 * Note:
 *   The definition of PageDirectory should be provided elsewhere.
 */
PageDirectory *createPageDirectoryNode(int pageNum);

#endif /* RECORD_MGR_H */