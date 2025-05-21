/************************************************************
 * File name:      interactive.c
 * Course:         CS 525 Advanced Database Organization (Spring 2025)
 * Authors:        Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This source file implements an interactive console interface for
 *   a simple student database using the Record Manager. It allows the user
 *   to create a table, view the table (functionality can be extended),
 *   insert student records, update student names, and delete student records.
 *   The interface uses a recursive menu system to repeatedly prompt the user
 *   for actions.
 ************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dberror.h"
#include "expr.h"
#include "record_mgr.h"
#include "tables.h"
#include "test_helper.h"

#define ID_LENGTH 0         // ID is an integer; no extra length required for string representation
#define NAME_LENGTH 10      // Maximum length for a student name

// Structure representing a student record for the interactive interface
typedef struct StudentRecord {
    int id;
    char *name;
} StudentRecord;

// Function prototypes
Record *asRecord(Schema *schema, StudentRecord record);
void interactiveCreate(char *fileName);
void interactiveView(void);
void interactiveInsert(int studentId, char *studentName);
void interactiveUpdate(int studentId, char *studentName);
void interactiveDelete(int studentId);

// Global variables for schema and table used in the interactive interface
Schema *interactiveSchema;
RM_TableData *interactiveTable;

/*
 * Function: recursive
 * -------------------
 * Recursively presents a menu to the user and handles input for various
 * operations such as creating a table, viewing the table, inserting, updating,
 * and deleting student records. The function calls itself at the end of each
 * operation to continuously prompt the user.
 */
void recursive() {
    printf("\n1. Create table\n"
           "2. View table\n"
           "3. Insert student\n"
           "4. Update student name\n"
           "5. Delete student\n"
           "\n"
           "V. View\n"
           "E. Exit\n"
           "\n"
           "What would you like to do:\n");
    char root[10]; // Increased buffer size to allow longer input
    scanf("%s", root);

    if (strcmp(root, "1") == 0) {
        printf("\nEnter table name:\n");
        char tableName[30];
        scanf("%s", tableName);
        interactiveCreate(tableName);
    } else if (strcmp(root, "2") == 0) {
        interactiveView();
    } else if (strcmp(root, "3") == 0) {
        printf("\nNew student ID:\n");
        int studentId;
        scanf("%d", &studentId);

        printf("\nNew student name:\n");
        char studentName[NAME_LENGTH];
        scanf("%s", studentName);

        interactiveInsert(studentId, studentName);
    } else if (strcmp(root, "4") == 0) {
        printf("\nExisting student ID:\n");
        int studentId;
        scanf("%d", &studentId);

        printf("\nChange student name:\n");
        char studentName[NAME_LENGTH];
        scanf("%s", studentName);

        interactiveUpdate(studentId, studentName);
    } else if (strcmp(root, "5") == 0) {
        printf("\nExisting student ID:\n");
        int studentId;
        scanf("%d", &studentId);

        interactiveDelete(studentId);
    } else if (strcmp(root, "e") == 0 || strcmp(root, "E") == 0) {
        printf("\nGoodbye!\n");
        closeTable(interactiveTable);
        exit(0);
    } else {
        printf("Unknown input!\n");
        closeTable(interactiveTable);
        exit(1);
    }

    // Recursive call to re-display the menu after each operation
    recursive();
}

/*
 * Function: main
 * --------------
 * The main entry point for the interactive program.
 * It displays a header and starts the recursive menu loop.
 *
 * Returns:
 *   0 on successful execution.
 */
int main() {
    printf("\nSTUDENTS DATABASE\n");
    recursive();
    return 0;
}

/*
 * Function: interactiveCreate
 * -----------------------------
 * Creates a new table with the specified file name. It defines a schema with
 * two attributes ("id" and "name"), creates the table on disk, and opens it
 * for further operations.
 *
 * Parameters:
 *   fileName - The name of the table (and underlying file) to create.
 */
void interactiveCreate(char *fileName) {
    char *names[] = {"id", "name"};
    DataType types[] = {DT_INT, DT_STRING};
    int sizes[] = {ID_LENGTH, NAME_LENGTH};
    int keys[] = {0};

    // Create a schema with two attributes
    interactiveSchema = createSchema(2, names, types, sizes, 1, keys);
    // Create the table on disk with the specified schema
    createTable(fileName, interactiveSchema);

    // Allocate and open the table for further operations
    interactiveTable = (RM_TableData *) malloc(sizeof(RM_TableData));
    openTable(interactiveTable, fileName);

    printf("Table created!\n");
}

/*
 * Function: interactiveView
 * ---------------------------
 * Displays the contents of the table. This function is currently a placeholder
 * and can be extended to iterate over records and print them.
 */
void interactiveView(void) {
    // TODO: Implement functionality to view table contents.
    printf("View table functionality is not yet implemented.\n");
}

/*
 * Function: asRecord
 * ------------------
 * Converts a StudentRecord into a Record structure according to the provided schema.
 *
 * Parameters:
 *   schema - The schema of the table.
 *   record - The StudentRecord containing student id and name.
 *
 * Returns:
 *   A pointer to a newly allocated Record containing the student's data.
 *
 * Notes:
 *   - This function creates a record using createRecord, then sets its attributes.
 */
Record *asRecord(Schema *schema, StudentRecord record) {
    Record *r = NULL;
    // Create a new record based on the schema
    if (createRecord(&r, schema) != RC_OK) {
        printf("Failed to create record\n");
        return NULL;
    }

    // Set the 'id' attribute (attribute index 0)
    Value *val;
    MAKE_VALUE(val, DT_INT, record.id);
    setAttr(r, schema, 0, val);
    free(val);

    // Set the 'name' attribute (attribute index 1)
    Value *val2;
    MAKE_STRING_VALUE(val2, record.name);
    setAttr(r, schema, 1, val2);
    free(val2);

    return r;
}

/*
 * Function: interactiveInsert
 * -----------------------------
 * Inserts a new student record into the table.
 *
 * Parameters:
 *   studentId   - The student's ID.
 *   studentName - The student's name.
 */
void interactiveInsert(int studentId, char *studentName) {
    StudentRecord record = {studentId, studentName};
    // Convert the StudentRecord to a Record and insert it into the table
    insertRecord(interactiveTable, asRecord(interactiveSchema, record));
    printf("Tuple inserted!\n");
}

/*
 * Function: interactiveUpdate
 * -----------------------------
 * Updates an existing student record in the table.
 *
 * Parameters:
 *   studentId   - The student's ID (used to locate the record).
 *   studentName - The new student name.
 *
 * Notes:
 *   - This simplistic implementation creates a new record from the given
 *     student data and uses it to update the record in the table.
 */
void interactiveUpdate(int studentId, char *studentName) {
    StudentRecord record = {studentId, studentName};
    updateRecord(interactiveTable, asRecord(interactiveSchema, record));
    printf("Tuple updated!\n");
}

/*
 * Function: interactiveDelete
 * -----------------------------
 * Deletes a student record from the table based on the provided student ID.
 *
 * Parameters:
 *   studentId - The student's ID, assumed to be stored as the page number
 *               in the record identifier.
 *
 * Notes:
 *   - Constructs a RID with the given studentId as the page number and a default
 *     slot value of 0 to identify the record to delete.
 */
void interactiveDelete(int studentId) {
    RID rid;
    rid.page = studentId;
    rid.slot = 0;  // Assumes slot 0 for simplicity; this may need adjustment.
    deleteRecord(interactiveTable, rid);
    printf("Tuple deleted!\n");
}