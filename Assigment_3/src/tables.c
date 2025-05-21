/************************************************************
 * File name:      tables.c
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This file implements utility functions for managing table
 *   schemas and table data. It includes functions to initialize
 *   a schema, initialize table metadata, and free allocated
 *   memory for a table.
 ************************************************************/

#include "tables.h"
#include <stdlib.h>
#include <string.h>

/*
 * Function: initSchema
 * --------------------
 * Initializes a Schema structure with a given number of attributes.
 *
 * Parameters:
 *   schema  - Pointer to the Schema structure to initialize.
 *   numAttr - The number of attributes in the schema.
 *
 * Description:
 *   Allocates memory for attribute names, data types, and attribute
 *   lengths based on the number of attributes.
 */
void initSchema(Schema *schema, int numAttr) {
    schema->numAttr = numAttr;
    schema->attrNames = malloc(sizeof(char *) * numAttr);
    schema->dataTypes = malloc(sizeof(DataType) * numAttr);
    schema->typeLength = malloc(sizeof(int) * numAttr);
}

/*
 * Function: initTableData
 * -----------------------
 * Initializes an RM_TableData structure for a table.
 *
 * Parameters:
 *   table  - Pointer to the RM_TableData structure to initialize.
 *   name   - The name of the table.
 *   schema - Pointer to the Schema structure describing the table.
 *
 * Description:
 *   Sets the table name (duplicating the provided string),
 *   assigns the schema, and initializes the management data pointer.
 */
void initTableData(RM_TableData *table, char *name, Schema *schema) {
    table->name = strdup(name);
    table->schema = schema;
    table->mgmtData = NULL;  // Connect to actual management data as needed
}

/*
 * Function: freeTableData
 * -----------------------
 * Frees all memory associated with an RM_TableData structure.
 *
 * Parameters:
 *   table - Pointer to the RM_TableData structure to free.
 *
 * Description:
 *   Frees the duplicated table name, all arrays within the schema,
 *   and the schema itself.
 */
void freeTableData(RM_TableData *table) {
    free(table->name);
    free(table->schema->attrNames);
    free(table->schema->dataTypes);
    free(table->schema->typeLength);
    free(table->schema);
}