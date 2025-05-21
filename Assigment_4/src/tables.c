/************************************************************
*     File name:                tables.c
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/
#include "tables.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "config.h"

/*
 * Initializes a Schema structure with the specified number of attributes.
 */
void initSchema(Schema *schema, int numAttr) {
    schema->numAttr = numAttr;
    schema->attrNames = malloc(sizeof(char *) * numAttr);
    schema->dataTypes = malloc(sizeof(DataType) * numAttr);
    schema->typeLength = malloc(sizeof(int) * numAttr);
}

/*
 * Initializes an RM_TableData structure with a table name and schema.
 */
void initTableData(RM_TableData *table, char *name, Schema *schema) {
    table->name = strdup(name);
    table->schema = schema;
    table->mgmtData = NULL;  // To be set by the record manager
}

/*
 * Frees the memory associated with a table's data.
 */
void freeTableData(RM_TableData *table) {
    free(table->name);
    free(table->schema->attrNames);
    free(table->schema->dataTypes);
    free(table->schema->typeLength);
    free(table->schema);
}

/*
 * Converts a string representation into a Value.
 * The first character indicates the type ('i', 'f', 'b', or 's').
 */
Value *stringToValue(char *val) {
    Value *result = (Value *) malloc(sizeof(Value));
    if (!result) return NULL;

    if (val[0] == 'i') {
        result->dt = DT_INT;
        sscanf(val + 1, "%d", &result->v.intV);
        printf("INT parsed from %s => %d\n", val, result->v.intV);

    } else if (val[0] == 'f') {
        result->dt = DT_FLOAT;
        sscanf(val + 1, "%f", &result->v.floatV);
    } else if (val[0] == 'b') {
        result->dt = DT_BOOL;
        result->v.boolV = (val[1] == 't') ? true : false;
    } else if (val[0] == 's') {
        result->dt = DT_STRING;
        result->v.stringV = strdup(val + 1);
    } else {
        free(result);
        return NULL;
    }

    return result;
}