/************************************************************
 *     File name:                record_mgr.h
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/
#ifndef RECORD_MGR_H
#define RECORD_MGR_H

#include "dberror.h"
#include "storage_mgr.h"
#include "buffer_mgr_stat.h"
#include "buffer_mgr.h"
#include "expr.h"
#include "tables.h"
#include "test_helper.h"
#include "rm_serializer.h"

#ifdef __cplusplus
extern "C" {
#endif


/* Scan handle for Record Manager */
typedef struct RM_ScanHandle {
   RM_TableData *rel;
   void *mgmtData;
} RM_ScanHandle;

/* Initialization and Shutdown */
extern RC initRecordManager(void *mgmtData);
extern RC shutdownRecordManager(void);

/* Table Operations */
extern RC createTable(char *name, Schema *schema);
extern RC openTable(RM_TableData *rel, char *name);
extern RC closeTable(RM_TableData *rel);
extern RC deleteTable(char *name);
extern int getNumTuples(RM_TableData *rel);
extern char *serializeTableInfo(RM_TableData *rel);

/* Record Operations */
extern RC insertRecord(RM_TableData *rel, Record *record);
extern RC deleteRecord(RM_TableData *rel, RID id);
extern RC updateRecord(RM_TableData *rel, Record *record);
extern RC getRecord(RM_TableData *rel, RID id, Record *record);

/* Scan Functions */
extern RC startScan(RM_TableData *rel, RM_ScanHandle *scan, Expr *cond);
extern RC next(RM_ScanHandle *scan, Record *record);
extern RC closeScan(RM_ScanHandle *scan);

/* Schema Management */
extern int getRecordSize(Schema *schema);
extern Schema *createSchema(int numAttr, char **attrNames, DataType *dataTypes, int *typeLength, int keySize, int *keys);
extern RC freeSchema(Schema *schema);
extern int attrOffset(Schema *schema, int attrNum, int *result);
    extern RC attrOffset(Schema *schema, int attrNum, int *offset);

/* Record and Attribute Handling */
extern RC createRecord(Record **record, Schema *schema);
extern RC freeRecord(Record *record);
extern RC getAttr(Record *record, Schema *schema, int attrNum, Value **value);
extern RC setAttr(Record *record, Schema *schema, int attrNum, Value *value);

/* Helper Function */
extern PageDirectory *createPageDirectoryNode(int pageNum);

#ifdef __cplusplus
}
#endif

#endif // RECORD_MGR_H