/************************************************************
 * File name:      rm_serializer.c
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This file implements the serialization and deserialization
 *   interfaces for the Record Manager. It provides functions to
 *   convert table metadata, schemas, records, page directories,
 *   and attribute values to their string representations and to
 *   parse them back into in-memory structures. These functions are
 *   used for persistent storage of metadata and for debugging.
 ************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "dberror.h"
#include "expr.h"
#include "record_mgr.h"
#include "tables.h"
#include "test_helper.h"
#include "rm_serializer.h"

/* ---------------------------------------------------------------------------
 * Serialization Functions
 * -------------------------------------------------------------------------*/

/*
 * Function: serializeTableInfo
 * ----------------------------
 * Serializes table metadata into a string.
 *
 * Parameters:
 *   rel - Pointer to the RM_TableData structure containing table info.
 *
 * Returns:
 *   A dynamically allocated string representing the table information.
 *   The caller is responsible for freeing the returned string.
 */
char * serializeTableInfo(RM_TableData *rel) {
    VarString *result;
    MAKE_VARSTRING(result);

    // Append basic table information
    APPEND(result, "TABLE <%s> with <%i> tuples:\n", rel->name, getNumTuples(rel));
    // Serialize the schema and append it
    char *schemaSerializeData = serializeSchema(rel->schema);
    APPEND_STRING(result, schemaSerializeData);
    free(schemaSerializeData);  // Free temporary schema string

    RETURN_STRING(result);
}

/*
 * Function: serializePageDirectories
 * ------------------------------------
 * Serializes a PageDirectoryCache structure into a string by serializing
 * each PageDirectory node.
 *
 * Parameters:
 *   pageDirectoryCache - Pointer to the PageDirectoryCache.
 *
 * Returns:
 *   A dynamically allocated string representing the page directories.
 */
char * serializePageDirectories(PageDirectoryCache *pageDirectoryCache) {
    VarString *result;
    MAKE_VARSTRING(result);
    PageDirectory *p = pageDirectoryCache->front;
    while(p != NULL) {
        APPEND_STRING(result, serializePageDirectory(p));
        p = p->next;
    }
    RETURN_STRING(result);
}

/*
 * Function: serializePageDirectory
 * ----------------------------------
 * Serializes a single PageDirectory node into a string.
 *
 * Parameters:
 *   pd - Pointer to the PageDirectory to serialize.
 *
 * Returns:
 *   A dynamically allocated string representing the page directory.
 */
char * serializePageDirectory(PageDirectory *pd) {
    VarString *result;
    MAKE_VARSTRING(result);
    int attrSize = sizeof(int);
    char data[attrSize + 1];
    // Initialize data buffer with '0's (assuming 4-digit fields)
    memset(data, '0', sizeof(char)*4);

    // Convert page number to string with fixed width (3 digits)
    PageInfoToString(3, pd->pageNum, data);
    APPEND(result, "[%s-", data);

    // Convert count of used slots
    memset(data, '0', sizeof(char)*4);
    PageInfoToString(3, pd->count, data);
    APPEND(result, "%s-", data);

    // Convert first free slot index
    memset(data, '0', sizeof(char)*4);
    PageInfoToString(3, pd->firstFreeSlot, data);
    APPEND(result, "%s]", data);
    APPEND_STRING(result, "\n");
    RETURN_STRING(result);
}

/*
 * Function: deserializePageDirectories
 * --------------------------------------
 * Deserializes a string into a PageDirectoryCache structure.
 *
 * Parameters:
 *   pdStr - The string representation of page directories.
 *
 * Returns:
 *   A pointer to a newly allocated PageDirectoryCache.
 */
PageDirectoryCache * deserializePageDirectories(char *pdStr) {
    PageDirectory *front = NULL;
    PageDirectory *rear = NULL;
    char *token;
    token = strtok(pdStr, "\n");
    int count = 0;
    while(token != NULL) {
        PageDirectory *pd = (PageDirectory*)malloc(sizeof(PageDirectory));
        char data[5];
        memset(data, '0', sizeof(char)*4);
        strncpy(data, token + 1, 4);
        data[4] = '\0';
        pd->pageNum = (int)strtol(data, NULL, 10);
        memset(data, '0', sizeof(char)*4);
        strncpy(data, token + 6, 4);
        data[4] = '\0';
        pd->count = (int)strtol(data, NULL, 10);
        memset(data, '0', sizeof(char)*4);
        strncpy(data, token + 11, 4);
        data[4] = '\0';
        pd->firstFreeSlot = (int)strtol(data, NULL, 10);
        pd->next = NULL;
        pd->pre = NULL;
        if(front == NULL) {
            front = pd;
        } else {
            rear->next = pd;
            pd->pre = rear;
        }
        rear = pd;
        token = strtok(NULL, "\n");
        count++;
    }
    PageDirectoryCache *pageDirectoryCache = (PageDirectoryCache*)malloc(sizeof(PageDirectoryCache));
    pageDirectoryCache->front = front;
    pageDirectoryCache->rear = rear;
    pageDirectoryCache->count = count;
    return pageDirectoryCache;
}

/*
 * Function: serializeSchema
 * -------------------------
 * Serializes a Schema structure into a string.
 *
 * Parameters:
 *   schema - Pointer to the Schema to serialize.
 *
 * Returns:
 *   A dynamically allocated string representing the schema.
 */
char * serializeSchema(Schema *schema) {
    int i;
    VarString *result;
    MAKE_VARSTRING(result);

    APPEND(result, "Schema with <%i> attributes (", schema->numAttr);

    // Append each attribute's name and data type
    for(i = 0; i < schema->numAttr; i++) {
        APPEND(result, "%s%s: ", (i != 0) ? ",": "", schema->attrNames[i]);
        switch (schema->dataTypes[i]) {
            case DT_INT:
                APPEND_STRING(result, "INT");
                break;
            case DT_FLOAT:
                APPEND_STRING(result, "FLOAT");
                break;
            case DT_STRING:
                APPEND(result, "STRING[%i]", schema->typeLength[i]);
                break;
            case DT_BOOL:
                APPEND_STRING(result, "BOOL");
                break;
        }
    }
    APPEND_STRING(result, ")");

    // Append key information
    APPEND_STRING(result, " with keys: {");
    for(i = 0; i < schema->keySize; i++)
        APPEND(result, "%s%s", ((i != 0) ? ",": ""), schema->attrNames[schema->keyAttrs[i]]);
    APPEND_STRING(result, "}\n");

    RETURN_STRING(result);
}

/*
 * Function: serializeTableContent
 * -------------------------------
 * Serializes all records of a table into a string.
 *
 * Parameters:
 *   rel - Pointer to the RM_TableData structure.
 *
 * Returns:
 *   A dynamically allocated string containing the serialized table content.
 */
char * serializeTableContent(RM_TableData *rel) {
    int i;
    VarString *result;
    RM_ScanHandle *sc = (RM_ScanHandle *) malloc(sizeof(RM_ScanHandle));
    Record *r = (Record *) malloc(sizeof(Record));
    MAKE_VARSTRING(result);

    // Append header with attribute names
    for(i = 0; i < rel->schema->numAttr; i++)
        APPEND(result, "%s%s", (i != 0) ? ", " : "", rel->schema->attrNames[i]);

    startScan(rel, sc, NULL);

    // Iterate over all records and append their serialized representation
    while(next(sc, r) != RC_RM_NO_MORE_TUPLES) {
        APPEND_STRING(result, serializeRecord(r, rel->schema));
        APPEND_STRING(result, "\n");
    }
    closeScan(sc);

    RETURN_STRING(result);
}

/*
 * Function: substring
 * -------------------
 * Extracts a substring from a given string between the first occurrence
 * of the start and end characters.
 *
 * Parameters:
 *   s     - The input string.
 *   start - The start delimiter character.
 *   end   - The end delimiter character.
 *
 * Returns:
 *   A newly allocated substring.
 */
char * substring(const char *s, const char start, const char end) {
    int i = 0, j = 0;
    while(s[i] != start) {
        i++;
    }
    i++;
    j = i;
    while(s[i] != end) {
        i++;
    }
    int size = i - j;
    char *temp = calloc(size + 1, sizeof(char));
    strncpy(temp, s + j, size);
    temp[size] = '\0';
    return temp;
}

/*
 * Function: deserializeSchema
 * ---------------------------
 * Deserializes a string into a Schema structure.
 *
 * Parameters:
 *   schemaData - The string representation of the schema.
 *
 * Returns:
 *   A pointer to the newly allocated Schema.
 */
Schema * deserializeSchema(char *schemaData) {
    Schema *schema = (Schema *) malloc(sizeof(Schema));

    char *numAttrStr = substring(schemaData, '<', '>');
    int numAttr = atoi(numAttrStr);
    schema->numAttr = numAttr;
    free(numAttrStr);

    // Extract attribute information enclosed in parentheses
    char *attrInfo = substring(schemaData, '(', ')');
    parseAttrInfo(schema, attrInfo);
    free(attrInfo);

    // Extract key information enclosed in curly braces
    char *keyInfo = substring(schemaData, '{', '}');
    parseKeyInfo(schema, keyInfo);
    free(keyInfo);

    return schema;
}

/*
 * Function: parseAttrInfo
 * -----------------------
 * Parses attribute information from a string and populates the schema.
 *
 * Parameters:
 *   schema   - Pointer to the Schema structure to populate.
 *   attrInfo - String containing attribute definitions.
 *
 * Returns:
 *   A pointer (unused) to the parsed attribute info (for compatibility).
 */
void * parseAttrInfo(Schema *schema, char *attrInfo) {
    char *t1;
    int numAttr = schema->numAttr;
    char **attrNames = (char **) malloc(sizeof(char*) * numAttr);
    DataType *dataTypes = (DataType *) malloc(sizeof(DataType) * numAttr);
    int *typeLength = (int *) malloc(sizeof(int) * numAttr);

    // Tokenize the attribute info string (format: "name: TYPE, ...")
    t1 = strtok(attrInfo, ": ");
    for(int i = 0; i < numAttr && t1 != NULL; ++i) {
        if(i != 0) {
            t1 = strtok(NULL, ": ");
        }
        int size = strlen(t1) + 1;
        attrNames[i] = (char *)malloc(size * sizeof(char));
        memcpy(attrNames[i], t1, size - 1);
        attrNames[i][size - 1] = '\0';

        // Get data type token
        t1 = strtok(NULL, ", ");
        if(t1[0] == 'I') {
            dataTypes[i] = DT_INT;
            typeLength[i] = 0;
        } else if(t1[0] == 'F') {
            dataTypes[i] = DT_FLOAT;
            typeLength[i] = 0;
        } else if(t1[0] == 'B') {
            dataTypes[i] = DT_BOOL;
            typeLength[i] = 0;
        } else if(t1[0] == 'S') {
            dataTypes[i] = DT_STRING;
        }
    }
    // Extract string length for string attributes from within square brackets
    char *length = substring(attrInfo, '[', ']');
    int stringLength = atoi(length);
    schema->attrNames = attrNames;
    for(int i = 0; i < numAttr; i++) {
        if(dataTypes[i] == DT_STRING) {
            typeLength[i] = stringLength;
        }
    }
    schema->dataTypes = dataTypes;
    schema->attrNames = attrNames;
    schema->typeLength = typeLength;
    free(length);
    return NULL;
}

/*
 * Function: parseKeyInfo
 * ----------------------
 * Parses key information from a string and populates the schema's key attributes.
 *
 * Parameters:
 *   schema  - Pointer to the Schema structure.
 *   keyInfo - String containing key attribute information.
 *
 * Returns:
 *   A pointer (unused) to the parsed key info (for compatibility).
 */
void * parseKeyInfo(Schema *schema, char *keyInfo) {
    int numAttr = schema->numAttr;
    int *keyAttrs = (int *) malloc(sizeof(int) * numAttr);

    // Find the index of the key attribute (assumes single key)
    int index = -1;
    for(int i = 0; i < numAttr; i++) {
        if(strcmp(schema->attrNames[i], keyInfo) == 0) {
            index = i;
            break;
        }
    }
    if(index == -1) {
        printf("Not a valid attribute name\n");
    }
    for(int i = 0; i < numAttr; ++i) {
        keyAttrs[i] = index;
    }
    schema->keyAttrs = keyAttrs;
    schema->keySize = 1;
    return NULL;
}

/*
 * Function: PageInfoToString
 * --------------------------
 * Converts an integer value (e.g., page number, count, or slot index)
 * to a fixed-width string representation.
 *
 * Parameters:
 *   j    - The width (number of digits) of the output string.
 *   val  - The integer value to convert.
 *   data - Buffer where the resulting string is stored.
 */
void PageInfoToString(int j, int val, char *data) {
    int r = 0;
    int q = val;
    int last = j;
    while (q > 0 && j >= 0) {
        r = q % 10;
        q = q / 10;
        data[j] = data[j] + r;
        j--;
    }
    data[last + 1] = '\0';
}

/*
 * Function: serializeRecord
 * -------------------------
 * Serializes a Record into a string based on the provided schema.
 *
 * Parameters:
 *   record - Pointer to the Record to serialize.
 *   schema - Pointer to the Schema that describes the record.
 *
 * Returns:
 *   A dynamically allocated string representing the serialized record.
 */
char * serializeRecord(Record *record, Schema *schema) {
    VarString *result;
    MAKE_VARSTRING(result);
    int i;
    int attrSize = sizeof(int);
    char data[attrSize + 1];
    memset(data, '0', sizeof(char) * 4);

    // Serialize record identifier: page number
    PageInfoToString(3, record->id.page, data);
    APPEND(result, "[%s", data);
    // Serialize record identifier: slot number
    memset(data, '0', sizeof(char) * 4);
    PageInfoToString(3, record->id.slot, data);
    APPEND(result, "-%s](", data);

    // Serialize each attribute value
    for(i = 0; i < schema->numAttr; i++) {
        APPEND_STRING(result, serializeAttr(record, schema, i));
        APPEND(result, "%s", (i == schema->numAttr - 1) ? "" : ",");
    }
    APPEND_STRING(result, ")\n");

    RETURN_STRING(result);
}

/*
 * Function: serializeAttr
 * -----------------------
 * Serializes a single attribute of a record into a string.
 *
 * Parameters:
 *   record  - Pointer to the Record.
 *   schema  - Pointer to the Schema.
 *   attrNum - The attribute index to serialize.
 *
 * Returns:
 *   A dynamically allocated string representing the serialized attribute.
 */
char * serializeAttr(Record *record, Schema *schema, int attrNum) {
    int offset;
    char attrData[5];
    VarString *result;
    MAKE_VARSTRING(result);

    attrOffset(schema, attrNum, &offset);
    // Copy fixed-length attribute data from the record
    memcpy(attrData, record->data + offset, 4);
    attrData[4] = '\0';

    switch(schema->dataTypes[attrNum]) {
        case DT_INT: {
            char val[5];
            memset(val, '\0', sizeof(val));
            memcpy(val, attrData, sizeof(int));
            APPEND(result, "%s:%s", schema->attrNames[attrNum], val);
        }
        break;
        case DT_STRING: {
            char *buf;
            int len = schema->typeLength[attrNum];
            buf = (char *) malloc(len + 1);
            strncpy(buf, attrData, len);
            buf[len] = '\0';
            APPEND(result, "%s:%s", schema->attrNames[attrNum], buf);
            free(buf);
        }
        break;
        case DT_FLOAT: {
            float val;
            memcpy(&val, attrData, sizeof(float));
            APPEND(result, "%s:%f", schema->attrNames[attrNum], val);
        }
        break;
        case DT_BOOL: {
            bool val;
            memcpy(&val, attrData, sizeof(bool));
            APPEND(result, "%s:%s", schema->attrNames[attrNum], val ? "TRUE" : "FALSE");
        }
        break;
        default:
            return "NO SERIALIZER FOR DATATYPE";
    }

    RETURN_STRING(result);
}

/*
 * Function: serializeValue
 * ------------------------
 * Serializes a Value into its string representation.
 *
 * Parameters:
 *   val - Pointer to the Value to serialize.
 *
 * Returns:
 *   A dynamically allocated string representing the value.
 */
char * serializeValue(Value *val) {
    VarString *result;
    MAKE_VARSTRING(result);

    switch(val->dt) {
        case DT_INT:
            APPEND(result, "%i", val->v.intV);
            break;
        case DT_FLOAT:
            APPEND(result, "%f", val->v.floatV);
            break;
        case DT_STRING:
            APPEND(result, "%s", val->v.stringV);
            break;
        case DT_BOOL:
            APPEND_STRING(result, (val->v.boolV ? "true" : "false"));
            break;
    }
    RETURN_STRING(result);
}

/*
 * Function: stringToValue
 * -----------------------
 * Converts a string representation of a value back into a Value structure.
 *
 * Parameters:
 *   val - The string representation, with a leading character indicating type.
 *
 * Returns:
 *   A pointer to the newly allocated Value.
 */
Value * stringToValue(char *val) {
    Value *result = (Value *) malloc(sizeof(Value));

    switch(val[0]) {
        case 'i':
            result->dt = DT_INT;
            result->v.intV = atoi(val + 1);
            break;
        case 'f':
            result->dt = DT_FLOAT;
            result->v.floatV = atof(val + 1);
            break;
        case 's':
            result->dt = DT_STRING;
            result->v.stringV = malloc(strlen(val));
            strcpy(result->v.stringV, val + 1);
            break;
        case 'b':
            result->dt = DT_BOOL;
            result->v.boolV = (val[1] == 't') ? TRUE : FALSE;
            break;
        default:
            result->dt = DT_INT;
            result->v.intV = -1;
            break;
    }
    return result;
}

/*
 * Function: attrOffset
 * --------------------
 * Calculates the byte offset for an attribute in a record, based on the schema.
 *
 * Parameters:
 *   schema  - Pointer to the Schema.
 *   attrNum - The attribute index.
 *   result  - Pointer to an integer where the computed offset is stored.
 *
 * Returns:
 *   RC_OK on success.
 */
RC attrOffset(Schema *schema, int attrNum, int *result) {
    int offset = 0;
    int attrPos = 0;

    for(attrPos = 0; attrPos < attrNum; attrPos++) {
        switch (schema->dataTypes[attrPos]) {
            case DT_STRING:
                offset += schema->typeLength[attrPos];
                break;
            case DT_INT:
                offset += sizeof(int);
                break;
            case DT_FLOAT:
                offset += sizeof(float);
                break;
            case DT_BOOL:
                offset += sizeof(bool);
                break;
        }
    }
    *result = offset;
    return RC_OK;
}

/* ---------------------------------------------------------------------------
 * Record Deserialization Functions
 * -------------------------------------------------------------------------*/

/*
 * Function: createRecordNode
 * --------------------------
 * Creates a new RecordNode for a deserialized record.
 *
 * Parameters:
 *   page       - The page number of the record.
 *   slot       - The slot index of the record.
 *   data       - The record's data as a string.
 *   sizeRecord - The expected size of the record.
 *
 * Returns:
 *   A pointer to the newly created RecordNode.
 */
RecordNode *createRecordNode(int page, int slot, char *data, int sizeRecord) {
    RecordNode *node = (RecordNode *)malloc(sizeof(RecordNode));
    char *content = (char*)calloc(sizeRecord + 1, sizeof(char));
    strcpy(content, data);
    content[strlen(content)] = '\0';
    node->page = page;
    node->slot = slot;
    node->data = data;
    node->pre = NULL;
    node->next = NULL;
    return node;
}

/*
 * Function: deserializeRecords
 * ----------------------------
 * Deserializes a string containing serialized records into a linked list
 * of RecordNode structures.
 *
 * Parameters:
 *   schema     - Pointer to the Schema.
 *   recordStr  - The string containing serialized records.
 *   sizeRecord - The size (in bytes) of each record.
 *
 * Returns:
 *   The head of a linked list of RecordNode structures.
 */
RecordNode * deserializeRecords(Schema *schema, char *recordStr, int sizeRecord) {
    if(recordStr == NULL || schema == NULL) {
        return NULL;
    }
    char *token;
    char *a = strdup(recordStr);
    token = strtok(a, "\n");

    RecordNode *head = NULL;
    RecordNode *p = NULL;
    while(token != NULL) {
        // Allocate a temporary Record and buffer for record data
        Record *newRecord = (Record*)malloc(sizeof(Record));
        char *recordData = (char*)calloc(sizeRecord, sizeof(char));
        newRecord->data = recordData;
        // Extract record identifier components (page and slot) and record data
        char pageNum[5];
        memset(pageNum, '\0', 4);
        strncpy(pageNum, token + 1, 4);
        pageNum[4] = '\0';
        int page = (int)strtol(pageNum, NULL, 10);
        char slotNum[5];
        memset(slotNum, '\0', 4);
        strncpy(slotNum, token + 6, 4);
        slotNum[4] = '\0';
        int slot = (int)strtol(slotNum, NULL, 10);
        char data[13];
        memset(data, '\0', 12);
        // Extract parts of the record data from fixed positions
        strncpy(data, token + 14, 4);
        strncpy(data + 4, token + 21, 4);
        strncpy(data + 8, token + 28, 4);
        data[12] = '\0';
        char *temp = strdup(data);
        RecordNode *node = createRecordNode(page, slot, temp, sizeRecord);
        if(head == NULL) {
            head = node;
        } else {
            p->next = node;
        }
        p = node;
        token = strtok(NULL, "\n");
    }
    free(a);
    return head;
}