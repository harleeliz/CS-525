/************************************************************
 * File name:      tables.h
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This header file defines the core data structures used by
 *   the Record Manager, including data types, records, schemas,
 *   and additional helper structures such as page directories
 *   for managing free slots. It also provides helper macros and
 *   declarations for serialization functions.
 ************************************************************/

#ifndef TABLES_H
#define TABLES_H

#include "dt.h"  // Data type definitions (e.g., bool)

/*
 * DataType Enumeration:
 * ---------------------
 * Defines the basic data types supported in the system.
 */
typedef enum DataType {
    DT_INT = 0,     // Integer type
    DT_STRING = 1,  // String type
    DT_FLOAT = 2,   // Floating point type
    DT_BOOL = 3     // Boolean type
} DataType;

/*
 * Structure: Value
 * ----------------
 * Represents a value with an associated data type.
 * The union 'v' stores the actual data based on the type.
 */
typedef struct Value {
    DataType dt;  // Data type of the value
    union v {
        int intV;       // Integer value
        char *stringV;  // String value (dynamically allocated)
        float floatV;   // Floating point value
        bool boolV;     // Boolean value
    } v;
} Value;

/*
 * Structure: RID (Record Identifier)
 * -----------------------------------
 * Uniquely identifies a record using a page number and slot.
 */
typedef struct RID {
    int page;  // Page number where the record is stored
    int slot;  // Slot number within the page
} RID;

/*
 * Structure: Record
 * -----------------
 * Represents a record in a table.
 *
 * Members:
 *   id   - Record identifier (RID)
 *   data - Binary representation of the record's attributes based on its schema
 */
typedef struct Record {
    RID id;
    char *data; // Binary data representing the record's attributes
} Record;

/*
 * Structure: Schema
 * -----------------
 * Describes the structure of a table.
 *
 * Members:
 *   numAttr    - Number of attributes in the schema
 *   attrNames  - Array of attribute names
 *   dataTypes  - Array of data types for each attribute
 *   typeLength - Array storing the size (length) for string attributes
 *   keyAttrs   - Array of indices indicating which attributes form the key
 *   keySize    - Total number of key attributes
 */
typedef struct Schema {
    int numAttr;
    char **attrNames;
    DataType *dataTypes;
    int *typeLength;
    int *keyAttrs;
    int keySize;
} Schema;

/*
 * Structure: RM_TableData
 * -----------------------
 * Management structure for a table used by the Record Manager.
 *
 * Members:
 *   name     - Name of the table
 *   schema   - Pointer to the table's schema
 *   mgmtData - Additional management data used internally by the Record Manager
 */
typedef struct RM_TableData {
    char *name;
    Schema *schema;
    void *mgmtData;
} RM_TableData;

/*
 * Structure: PageDirectory
 * ------------------------
 * Used for managing free slots within a page.
 *
 * Members:
 *   pageNum       - Index of the page
 *   count         - Number of free slots available in the page
 *   firstFreeSlot - Location (index) of the first free slot in the page
 *   pre, next     - Pointers to previous and next page directory nodes (for doubly-linked list)
 */
typedef struct PageDirectory {
    int pageNum;
    int count;
    int firstFreeSlot;
    struct PageDirectory *pre;
    struct PageDirectory *next;
} PageDirectory;

/*
 * Structure: PageDirectoryCache
 * -----------------------------
 * Cache structure to store multiple PageDirectory nodes.
 *
 * Members:
 *   count    - Current number of stored page directory nodes
 *   capacity - Maximum capacity of the cache
 *   front    - Pointer to the first node in the cache
 *   rear     - Pointer to the last node in the cache
 */
typedef struct PageDirectoryCache {
    int count;
    int capacity;
    PageDirectory *front;
    PageDirectory *rear;
} PageDirectoryCache;

/*
 * Structure: RecordNode
 * ---------------------
 * Doubly-linked list node for managing records.
 *
 * Members:
 *   page  - Page number where the record is stored
 *   slot  - Slot number of the record in the page
 *   data  - Binary representation of the record's data
 *   pre, next - Pointers to the previous and next record nodes
 */
typedef struct RecordNode {
    int page;
    int slot;
    char *data;
    struct RecordNode *pre;
    struct RecordNode *next;
} RecordNode;

/*
 * Macro: MAKE_STRING_VALUE
 * ------------------------
 * Allocates and initializes a Value of type DT_STRING.
 *
 * Parameters:
 *   result - Variable to hold the allocated Value pointer.
 *   value  - C-string to be stored in the Value.
 */
#define MAKE_STRING_VALUE(result, value)              \
    do {                                              \
        (result) = (Value *) malloc(sizeof(Value));   \
        (result)->dt = DT_STRING;                     \
        (result)->v.stringV = (char *) malloc(strlen(value) + 1); \
        strcpy((result)->v.stringV, value);           \
    } while(0)

/*
 * Macro: MAKE_VALUE
 * -----------------
 * Allocates and initializes a Value of the specified datatype.
 *
 * Parameters:
 *   result   - Variable to hold the allocated Value pointer.
 *   datatype - DataType for the Value.
 *   value    - The value to assign (for DT_INT, DT_FLOAT, or DT_BOOL).
 */
#define MAKE_VALUE(result, datatype, value)           \
    do {                                              \
        (result) = (Value *) malloc(sizeof(Value));   \
        (result)->dt = datatype;                      \
        switch(datatype) {                            \
            case DT_INT:                              \
                (result)->v.intV = value;             \
                break;                                \
            case DT_FLOAT:                            \
                (result)->v.floatV = value;           \
                break;                                \
            case DT_BOOL:                             \
                (result)->v.boolV = value;            \
                break;                                \
        }                                             \
    } while(0)

/*
 * Serialization and Debug Functions:
 * ------------------------------------
 * The following functions are declared for debugging and for converting
 * tables, schemas, records, and values to their string representations.
 */

/* Converts a string into a Value of type DT_STRING. */
extern Value *stringToValue (char *value);

/* Serializes table metadata into a string representation. */
extern char *serializeTableInfo(RM_TableData *rel);

/* Serializes the content (records) of a table into a string. */
extern char *serializeTableContent(RM_TableData *rel);

/* Serializes a schema into a string representation. */
extern char *serializeSchema(Schema *schema);

/* Serializes a record into a string representation based on the provided schema. */
extern char *serializeRecord(Record *record, Schema *schema);

/* Serializes a specific attribute of a record into a string. */
extern char *serializeAttr(Record *record, Schema *schema, int attrNum);

/* Serializes a Value into its string representation. */
extern char *serializeValue(Value *val);

#endif