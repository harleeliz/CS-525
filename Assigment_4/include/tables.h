/************************************************************
 *     File name:                tables.h
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/
#ifndef TABLES_H
#define TABLES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dt.h"  // Defines bool and DataType

/* --- Data Types, Records, and Schemas --- */

/* Supported data types */
    typedef enum DataType {
        DT_INT = 0,
        DT_STRING,
        DT_FLOAT,
        DT_BOOL
    } DataType;

/* A Value stores one data item. */
    typedef struct Value {
        DataType dt;
        union v {
            int intV;
            float floatV;
            bool boolV;
            char *stringV;
        } v;
    } Value;

/* Record Identifier (RID) */
typedef struct RID {
    int page;
    int slot;
} RID;

/* A Record consists of its RID and raw data (a serialized record). */
typedef struct Record {
    RID id;
    char *data;
} Record;

/* Schema defines a table’s structure. */
typedef struct Schema {
    int numAttr;
    char **attrNames;
    DataType *dataTypes;
    int *typeLength;    // Only relevant for string types
    int *keyAttrs;      // Array of attribute indexes that form the key
    int keySize;
} Schema;

/* Table data structure: stores a table's name, its schema, and manager-specific data. */
typedef struct RM_TableData {
    char *name;
    Schema *schema;
    void *mgmtData;
} RM_TableData;

/* --- Auxiliary Structures --- */

/* PageDirectory: used to track free slots on a page */
typedef struct PageDirectory {
    int pageNum;
    int count;
    int firstFreeSlot;
    struct PageDirectory *pre;
    struct PageDirectory *next;
} PageDirectory;

/* Cache for page directories */
typedef struct PageDirectoryCache {
    int count;
    int capacity;
    PageDirectory *front;
    PageDirectory *rear;
} PageDirectoryCache;

/* RecordNode: used for linking records in memory */
typedef struct RecordNode {
    int page;
    int slot;
    char *data;
    struct RecordNode *pre;
    struct RecordNode *next;
} RecordNode;

/* --- Macros to Create Values --- */

#define MAKE_STRING_VALUE(result, value)                \
do {                                                    \
    (result) = (Value*) malloc(sizeof(Value));          \
    (result)->dt = DT_STRING;                           \
    (result)->v.stringV = strdup(value);                \
} while (0)

#define MAKE_INT_VALUE(result, value)                   \
do {                                                    \
    (result) = (Value *) malloc(sizeof(Value));         \
    (result)->dt = DT_INT;                              \
    (result)->v.intV = value;                           \
} while (0)

#define MAKE_BOOL_VALUE(result, value)                  \
do {                                                    \
    (result) = (Value*) calloc(sizeof(Value));          \
    (result)->dt = DT_BOOL;                             \
    (result)->v.boolV = value;                          \
} while (0)

#define MAKE_FLOAT_VALUE(result, value)                 \
do {                                                    \
    (result) = (Value*) malloc(sizeof(Value));          \
    (result)->dt = DT_FLOAT;                            \
    (result)->v.floatV = value;                         \
} while (0)

/* --- Serialization and Debug Functions --- */
extern Value *stringToValue(char *value);
extern char *serializeTableInfo(RM_TableData *rel);
extern char *serializeTableContent(RM_TableData *rel);
extern char *serializeSchema(Schema *schema);
extern char *serializeRecord(Record *record, Schema *schema);
extern char *serializeAttr(Record *record, Schema *schema, int attrNum);
extern char *serializeValue(Value *val);

#ifdef __cplusplus
}
#endif

#endif // TABLES_H