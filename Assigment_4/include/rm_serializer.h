/************************************************************
 * File name:      rm_serializer.h
 * CS 525 Advanced Database Organization (Spring 2025)
 * Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This header file defines the serialization and deserialization
 *   interfaces for the Record Manager. It includes macros for
 *   managing dynamic strings, which are used to build string
 *   representations of table information, schemas, records, page
 *   directories, and more. It also declares functions to calculate
 *   attribute offsets and to convert various data structures to and
 *   from their string representations.
 ************************************************************/

#ifndef RM_SERIALIZER_H
#define RM_SERIALIZER_H

#include "dberror.h"
#include "tables.h"

/************************************************************
 *                    Handle Data Structures              *
 ************************************************************/

/*
 * Structure: VarString
 * --------------------
 * A dynamic string structure used to build and store string
 * representations. It maintains a buffer, the current string size,
 * and the total allocated buffer size.
 */
typedef struct VarString {
    char *buf;    // Pointer to the character buffer
    int size;     // Current length of the string
    int bufsize;  // Total capacity of the buffer
} VarString;

/*
 * Macro: MAKE_VARSTRING
 * ---------------------
 * Allocates and initializes a VarString with an initial buffer size
 * of 100 bytes.
 *
 * Parameters:
 *   var - Variable to hold the pointer to the newly allocated VarString.
 */
#define MAKE_VARSTRING(var)                           \
    do {                                              \
        var = (VarString *) malloc(sizeof(VarString));\
        var->size = 0;                                \
        var->bufsize = 100;                           \
        var->buf = malloc(100);                       \
    } while (0)

/*
 * Macro: FREE_VARSTRING
 * ---------------------
 * Frees the memory allocated for a VarString, including its buffer.
 *
 * Parameters:
 *   var - The VarString pointer to be freed.
 */
#define FREE_VARSTRING(var)                           \
    do {                                              \
        free(var->buf);                               \
        free(var);                                    \
    } while (0)

/*
 * Macro: GET_STRING
 * -----------------
 * Copies the content of a VarString into a newly allocated null-
 * terminated string.
 *
 * Parameters:
 *   result - Variable to hold the resulting C-string.
 *   var    - The VarString pointer to copy from.
 */
#define GET_STRING(result, var)                       \
    do {                                              \
        result = malloc((var->size) + 1);             \
        memcpy(result, var->buf, var->size);          \
        result[var->size] = '\0';                     \
    } while (0)

/*
 * Macro: RETURN_STRING
 * --------------------
 * Convenience macro that extracts a C-string from a VarString,
 * frees the VarString, and returns the resulting string.
 *
 * Parameters:
 *   var - The VarString pointer to convert and free.
 */
#define RETURN_STRING(var)                            \
    do {                                              \
        char *resultStr;                              \
        GET_STRING(resultStr, var);                   \
        FREE_VARSTRING(var);                          \
        return resultStr;                             \
    } while (0)

/*
 * Macro: ENSURE_SIZE
 * ------------------
 * Ensures that the VarString has enough buffer size to accommodate
 * new data. If the current buffer size is insufficient, it doubles
 * the buffer size until it meets the new size requirement.
 *
 * Parameters:
 *   var     - The VarString pointer to check.
 *   size_t- The new required size.
 */
#define ENSURE_SIZE(var, newsize)                             \
if ((size_t)(var)->bufsize < (size_t)(newsize)) {         \
int newbufsize = (var)->bufsize;                      \
while ((size_t)(newbufsize * 2) < (size_t)(newsize))  \
newbufsize *= 2;                                  \
(var)->buf = realloc((var)->buf, newbufsize);         \
(var)->bufsize = newbufsize;                          \
}



/*
 * Macro: APPEND_STRING
 * --------------------
 * Appends a given C-string to the VarString. It ensures that the
 * VarString has sufficient capacity, then copies the new string
 * into the buffer and updates the size.
 *
 * Parameters:
 *   var    - The VarString pointer to append to.
 *   string - The C-string to append.
 */
#define APPEND_STRING(var, string)                    \
    do {                                              \
        ENSURE_SIZE(var, var->size + (int)strlen(string)); \
        memcpy(var->buf + var->size, string, strlen(string)); \
        var->size += strlen(string);                  \
    } while(0)

/*
 * Macro: APPEND
 * -------------
 * Appends formatted data (using printf-style formatting) to the VarString.
 *
 * Parameters:
 *   var - The VarString pointer to append to.
 *   ... - Formatted data (format string and arguments).
 *
 * This macro uses a temporary buffer to format the data, appends it to
 * the VarString, and then frees the temporary buffer.
 */
#define APPEND(var, ...)                              \
    do {                                              \
        char *tmp = malloc(10000);                    \
        sprintf(tmp, __VA_ARGS__);                    \
        APPEND_STRING(var, tmp);                      \
        free(tmp);                                    \
    } while(0)

/************************************************************
 *                    Interface Functions                 *
 ************************************************************/

/* Serialization Functions */

/* Serializes table metadata into a string representation. */
extern char * serializeTableInfo(RM_TableData *rel);

/* Serializes the content (records) of a table into a string representation. */
extern char * serializeTableContent(RM_TableData *rel);

/* Serializes a schema into a string representation. */
extern char * serializeSchema(Schema *schema);

/* Serializes a record into a string representation based on the schema. */
extern char * serializeRecord(Record *record, Schema *schema);

/* Serializes a specific attribute of a record into a string. */
extern char * serializeAttr(Record *record, Schema *schema, int attrNum);

/* Serializes a Value into its string representation. */
extern char * serializeValue(Value *val);

/* Serializes a PageDirectory structure into a string. */
extern char * serializePageDirectory(PageDirectory *pd);

/* Serializes a PageDirectoryCache (multiple PageDirectory nodes) into a string. */
extern char * serializePageDirectories(PageDirectoryCache *tableCache);

/* Deserialization Functions */

/* Deserializes table information from a string and populates the RM_TableData structure. */
extern void * deserializeTableInfo(RM_TableData *rel, char *tableInfo);

/* Deserializes a schema from its string representation. */
extern Schema * deserializeSchema(char *schemaData);

/* Deserializes page directories from a string and returns a PageDirectoryCache structure. */
extern PageDirectoryCache * deserializePageDirectories(char *pdStr);

/* Deserializes records from a string representation based on the schema. */
extern RecordNode * deserializeRecords(Schema *schema, char *recordStr, int size);

/* Converts a string to a Value structure. */
extern Value * stringToValue(char *val);

/* Helper Functions */

/* Returns a substring from a given string, starting at character 'start' and ending at character 'end'. */
extern char * substring(const char *s, const char start, const char end);

/* Parses attribute information from a string based on the provided schema. */
extern void * parseAttrInfo(Schema *schema, char *attrInfo);

/* Parses key information from a string based on the provided schema. */
extern void * parseKeyInfo(Schema *schema, char *keyInfo);

/* Parses a PageDirectory structure from its string representation. */
extern PageDirectory * parsePageDirectory(char *t);

/* Parses a record from a tokenized string representation based on the schema. */
extern void parseRecord(Schema *schema, Record *record, char *token);

/* Converts page information to a string representation.
 * Parameters:
 *   j    - Page index.
 *   val  - Value associated with the page.
 *   data - Output buffer to store the resulting string.
 */
void PageInfoToString(int j, int val, char *data);

#endif
