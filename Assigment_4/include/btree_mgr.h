/************************************************************
 * File name:  btree_mgr.h
 * CS 525 Advanced Database Organization (Spring 2025)
 * Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 * Description:
 *   This header file defines the Buffer Manager interface,
 *   data types, structures, and helper functions used for
 *   managing the buffer pool and page cache.
 ************************************************************/

#ifndef BTREE_MGR_H
#define BTREE_MGR_H

#include "dberror.h"
#include "tables.h"

/* BTreeHandle: Structure to manage a B⁺‑tree index.
 *  - keyType: Type of keys (only DT_INT supported).
 *  - idxId: Unique identifier (e.g., file name).
 *  - mgmtData: Pointer to internal management data.
 */
typedef struct BTreeHandle {
  DataType keyType;
  char *idxId;
  void *mgmtData;
} BTreeHandle;

/* BT_ScanHandle: Structure to manage in‑order scans of a B⁺‑tree.
 *  - tree: Pointer to the associated BTreeHandle.
 *  - mgmtData: Pointer to scan-specific data.
 */
typedef struct BT_ScanHandle {
  BTreeHandle *tree;
  void *mgmtData;
} BT_ScanHandle;

/* Index Manager Functions */
extern RC initIndexManager(void *mgmtData);   // Initialize the index manager
extern RC shutdownIndexManager(void);         // Shutdown the index manager

/* B⁺‑tree Creation and Destruction */
extern RC createBtree(char *idxId, DataType keyType, int n);  // Create a new B⁺‑tree
extern RC openBtree(BTreeHandle **tree, char *idxId);         // Open an existing B⁺‑tree
extern RC closeBtree(BTreeHandle *tree);                      // Close the B⁺‑tree (flush changes)
extern RC deleteBtree(char *idxId);                           // Delete the B⁺‑tree index

/* B⁺‑tree Information Access */
extern RC getNumNodes(BTreeHandle *tree, int *result);        // Get total node count
extern RC getNumEntries(BTreeHandle *tree, int *result);      // Get total key entry count
extern RC getKeyType(BTreeHandle *tree, DataType *result);      // Get the key type

/* Key Operations */
extern RC findKey(BTreeHandle *tree, Value *key, RID *result);  // Find key; return corresponding RID
extern RC insertKey(BTreeHandle *tree, Value *key, RID rid);    // Insert key/RID pair
extern RC deleteKey(BTreeHandle *tree, Value *key);             // Delete key and its RID

/* Tree Scanning Functions */
extern RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle); // Start an in‑order scan
extern RC nextEntry(BT_ScanHandle *handle, RID *result);           // Get the next RID in the scan
extern RC closeTreeScan(BT_ScanHandle *handle);                    // Close the scan

/* Debug Function */
extern char *printTree(BTreeHandle *tree);  // Return a string representing the B⁺‑tree (depth‑first pre‑order)

#endif // BTREE_MGR_H
