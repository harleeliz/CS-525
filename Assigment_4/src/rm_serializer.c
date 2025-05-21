/************************************************************
 *     File name:                rm_serializer.c
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/
#include <stdio.h>
#include <string.h>
#include "dberror.h"
#include "record_mgr.h"
#include "tables.h"
#include "test_helper.h"
#include "rm_serializer.h"


/* --- Page Information Formatting --- */

/**
 * Converts an integer value into a fixed-width string.
 *
 * @param width  The fixed width of the output.
 * @param value  The integer value to format.
 * @param data   The output buffer.
 */
void PageInfoToString(int width, int value, char *data) {
    snprintf(data, width + 1, "%0*d", width, value);
}

/* --- Serialization Functions --- */

/**
 * Serializes a Value into a string.
 * Returns a dynamically allocated string that should be freed by the caller.
 */
char *serializeValue(Value *val) {
    char *result = (char *) malloc(50);
    if (!result) return NULL;

    switch (val->dt) {
        case DT_INT:
            sprintf(result, "%d", val->v.intV);
            break;
        case DT_FLOAT:
            sprintf(result, "%f", val->v.floatV);
            break;
        case DT_BOOL:
            sprintf(result, "%s", val->v.boolV ? "true" : "false");
            break;
        case DT_STRING:
            sprintf(result, "%s", val->v.stringV);
            break;
        default:
            free(result);
            return NULL;
    }
    return result;
}

/**
 * Serializes a Record into a string.
 * The returned string should be freed by the caller.
 */
char *serializeRecord(Record *record, Schema *schema) {
    (void)schema;
    char *result = (char *) malloc(100);
    if (!result) return NULL;
    snprintf(result, 100, "%d:%d:%s", record->id.page, record->id.slot, record->data);
    return result;
}

/**
 * Serializes the content of a table.
 * Reads through the table using a scan and returns a string representation.
 */
char *serializeTableContent(RM_TableData *rel) {
    RM_ScanHandle scan;
    Record *record;
    char *result = malloc(PAGE_SIZE);
    int offset = 0;

    if (!result) return NULL;
    result[0] = '\0';

    if (startScan(rel, &scan, NULL) != RC_OK) {
        free(result);
        return NULL;
    }
    if (createRecord(&record, rel->schema) != RC_OK) {
        closeScan(&scan);
        free(result);
        return NULL;
    }

    while (next(&scan, record) != RC_RM_NO_MORE_TUPLES) {
        char *serializedRecord = serializeRecord(record, rel->schema);
        if (!serializedRecord) {
            freeRecord(record);
            closeScan(&scan);
            free(result);
            return NULL;
        }
        offset += snprintf(result + offset, PAGE_SIZE - offset, "%s\n", serializedRecord);
        free(serializedRecord);
        if (offset >= PAGE_SIZE) break;
    }

    freeRecord(record);
    closeScan(&scan);
    return result;
}

/**
 * Serializes a PageDirectory.
 * Returns a dynamically allocated string representing the page directory.
 */
char *serializePageDirectory(PageDirectory *pd) {
    VarString *result;
    MAKE_VARSTRING(result);
    int attrSize = sizeof(int);
    char data[attrSize + 1];
    memset(data, '0', sizeof(char) * 4);

    PageInfoToString(3, pd->pageNum, data);
    APPEND(result, "[%s-", data);

    memset(data, '0', sizeof(char) * 4);
    PageInfoToString(3, pd->count, data);
    APPEND(result, "%s-", data);

    memset(data, '0', sizeof(char) * 4);
    PageInfoToString(3, pd->firstFreeSlot, data);
    APPEND(result, "%s]", data);

    APPEND_STRING(result, "\n");
    RETURN_STRING(result);
}

/**
 * Serializes all page directories from a cache.
 */
char *serializePageDirectories(PageDirectoryCache *pageDirectoryCache) {
    VarString *result;
    MAKE_VARSTRING(result);
    PageDirectory *p = pageDirectoryCache->front;
    while (p != NULL) {
        APPEND_STRING(result, serializePageDirectory(p));
        p = p->next;
    }
    RETURN_STRING(result);
}

/**
 * Serializes a Schema into a human-readable string.
 */
char *serializeSchema(Schema *schema) {
    VarString *result;
    MAKE_VARSTRING(result);
    int i;

    APPEND(result, "Schema with <%i> attributes (", schema->numAttr);
    for (i = 0; i < schema->numAttr; i++) {
        APPEND(result, "%s%s: ", (i != 0) ? ", " : "", schema->attrNames[i]);
        switch (schema->dataTypes[i]) {
            case DT_INT:    APPEND_STRING(result, "INT"); break;
            case DT_FLOAT:  APPEND_STRING(result, "FLOAT"); break;
            case DT_STRING: APPEND(result, "STRING[%i]", schema->typeLength[i]); break;
            case DT_BOOL:   APPEND_STRING(result, "BOOL"); break;
        }
    }
    APPEND_STRING(result, ") with keys: {");
    for (i = 0; i < schema->keySize; i++)
        APPEND(result, "%s%s", (i != 0) ? ", " : "", schema->attrNames[schema->keyAttrs[i]]);
    APPEND_STRING(result, "}\n");

    RETURN_STRING(result);
}


Schema* deserializeSchema(char *schemaData) {
    (void)schemaData;
    // Implement your schema deserialization logic here.
    // For example, allocate a Schema, parse the schemaData string,
    // fill the Schema fields, and return the pointer.
    Schema *schema = malloc(sizeof(Schema));
    if (!schema) return NULL;
    // ... parse schemaData to initialize schema ...
    return schema;
}