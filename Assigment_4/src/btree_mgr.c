/************************************************************
*     File name:                btree_mgr.c
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 *
 *  A simplified in-memory B⁺‑tree implementation for integer keys.
 *  Each node occupies one “page” (conceptually) and supports basic
 *  insertion and search. Persistence is simulated via the storage
 *  manager functions.
 ************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "btree_mgr.h"
#include "dberror.h"
#include "storage_mgr.h"
#include "tables.h"


/* ============================================================================
 * Internal Data Structures for B+ Tree Nodes
 * ============================================================================
 */

/* NodeType distinguishes Internal vs Leaf Nodes */
typedef enum NodeType {
    INTERNAL,   // Non-leaf node containing keys and child pointers
    LEAF        // Leaf node containing keys and RIDs
} NodeType;

/* Core BTreeNode structure (each node fits in one "page") */
typedef struct BTreeNode {
    NodeType type;    // INTERNAL or LEAF
    int numKeys;      // Current number of keys stored
    int *keys;        // Array of keys in sorted order

    union {
        struct BTreeNode **children; // For INTERNAL nodes: array of child pointers
        RID *rids;                   // For LEAF nodes: array of Record IDs (RIDs)
    } pointers;

    struct BTreeNode *next;  // For LEAF nodes: pointer to next leaf (linked list for scanning)
} BTreeNode;

/* Structure for Managing the Whole B+ Tree Metadata */
typedef struct BTreeMgmtData {
    int order;           // Maximum number of keys allowed in a node
    BTreeNode *root;     // Pointer to the root node of the tree
    int numNodes;        // Total number of nodes in the tree
    int numEntries;      // Total number of entries (keys) stored in the tree
} BTreeMgmtData;

/* ============================================================================
 * Helper Functions - Node Creation
 * ============================================================================
 */

/* Create an Empty Leaf Node */
static BTreeNode* createLeafNode(int order) {
    BTreeNode *node = (BTreeNode*) malloc(sizeof(BTreeNode));
    if (!node) return NULL;
    node->type = LEAF;
    node->numKeys = 0;
    node->keys = (int*) malloc(sizeof(int) * order);
    node->pointers.rids = (RID*) malloc(sizeof(RID) * order);
    node->next = NULL;
    printf("Created leaf node\n");
    return node;
}


/* Create an Empty Internal Node */
static BTreeNode* createInternalNode(int order) {
    BTreeNode *node = (BTreeNode*) malloc(sizeof(BTreeNode));
    if (!node) return NULL;
    node->type = INTERNAL;
    node->numKeys = 0;
    node->keys = (int*) malloc(sizeof(int) * order);
    node->pointers.children = (BTreeNode**) malloc(sizeof(BTreeNode*) * (order + 1));
    node->next = NULL;
    return node;
}

/* ============================================================================
 * Helper Function - Recursive Search to Locate Leaf Node for Given Key
 * ============================================================================
 *
 * Traverses the tree from the given node to the correct leaf node where
 * the key would either exist or should be inserted.
 */
static BTreeNode* findLeaf(BTreeNode *node, int key) {
    if (!node) return NULL; // If it's already a leaf, we are done
    if (node->type == LEAF)
        return node;
    int i = 0; // Traverse the correct child by comparing keys
    while (i < node->numKeys && key >= node->keys[i])
        i++;
    return findLeaf(node->pointers.children[i], key);
}

/* ============================================================================
 * Insert a Key-RID Pair into a Leaf Node without Splitting
 * ============================================================================
 *
 * Assumes the leaf has enough space for the new key.
 * Inserts the key maintaining sorted order.
 */
static void insertIntoLeaf(BTreeNode *leaf, int key, RID rid, int order) {
    (void)*leaf;
    (void)key;
    (void)rid;
    (void)order;
    int i;
    // Find position to insert to keep keys sorted
    for (i = leaf->numKeys - 1; i >= 0 && leaf->keys[i] > key; i--) {
        leaf->keys[i+1] = leaf->keys[i];
        leaf->pointers.rids[i+1] = leaf->pointers.rids[i];
    }
    leaf->keys[i+1] = key;
    leaf->pointers.rids[i+1] = rid;
    leaf->numKeys++;
}

/* ============================================================================
 * Split a Full Leaf Node and Insert a New Key-RID Pair
 * ============================================================================
 *
 * Promotes the smallest key in the new right-split leaf to the parent.
 * Returns pointer to the new right-split leaf.
 */
static BTreeNode* splitLeaf(BTreeNode *leaf, int order, int key, RID rid, int *promotedKey) {
    // Create temporary arrays to hold keys and RIDs (size = current keys + 1)
    int total = leaf->numKeys + 1;
    int *tempKeys = (int*) malloc(sizeof(int) * total);
    RID *tempRids = (RID*) malloc(sizeof(RID) * total);
    int i, j = 0;
    int inserted = 0;
    // Merge existing keys and new key in sorted order
    for (i = 0; i < leaf->numKeys; i++) {
        if (!inserted && key < leaf->keys[i]) {
            tempKeys[j] = key;
            tempRids[j] = rid;
            j++;
            inserted = 1;
        }
        tempKeys[j] = leaf->keys[i];
        tempRids[j] = leaf->pointers.rids[i];
        j++;
    }
    if (!inserted) {
        tempKeys[j] = key;
        tempRids[j] = rid;
        j++;
    }
    // Determine split point based on the assignment’s convention:
    int split;
    if (order % 2 == 0)
        split = (total + 1) / 2;  // Left node gets the extra key when order is even
    else
        split = total / 2;        // Even split when order is odd

    // Create new leaf node
    BTreeNode *newLeaf = createLeafNode(order);
    // Copy first 'split' keys to original leaf
    leaf->numKeys = split;
    for (i = 0; i < split; i++) {
        leaf->keys[i] = tempKeys[i];
        leaf->pointers.rids[i] = tempRids[i];
    }
    // Copy remaining keys to new leaf
    newLeaf->numKeys = total - split;
    for (i = 0; i < newLeaf->numKeys; i++) {
        newLeaf->keys[i] = tempKeys[split + i];
        newLeaf->pointers.rids[i] = tempRids[split + i];
    }
    // Update leaf links for scanning
    newLeaf->next = leaf->next;
    leaf->next = newLeaf;
    *promotedKey = newLeaf->keys[0]; // Promote the smallest key in the new leaf to parent
    free(tempKeys);
    free(tempRids);
    return newLeaf;
}

/* ============================================================================
 * Recursive Insertion into a Subtree
 * ============================================================================
 *
 * Handles insertion and propagates splits up the tree as necessary.
 * If a split occurs, *promotedKey contains the key to insert in parent,
 * and *newChild points to the new right child node.
 */
static BTreeNode* insertRecursive(BTreeNode *node, int key, RID rid, int order, int *promotedKey, BTreeNode **newChild) {
    if (node->type == LEAF) {
        if (node->numKeys < order) {
            insertIntoLeaf(node, key, rid, order);
            *promotedKey = -1;
            *newChild = NULL;
            return node;
        } else {
            BTreeNode *newLeaf = splitLeaf(node, order, key, rid, promotedKey);
            *newChild = newLeaf;
            return node;
        }
    } else {  // INTERNAL node
        int i = 0;
        while (i < node->numKeys && key >= node->keys[i])
            i++;
        int childPromotedKey;
        BTreeNode *childNew = NULL;

        // Recursive insert into correct child
        node->pointers.children[i] = insertRecursive(node->pointers.children[i], key, rid, order, &childPromotedKey, &childNew);
        if (childNew == NULL) {
            *promotedKey = -1;
            *newChild = NULL;
            return node;
        }
        // Insert promoted key and new child pointer into internal node
        if (node->numKeys < order) {
            int j;
            for (j = node->numKeys - 1; j >= i; j--) {
                node->keys[j+1] = node->keys[j];
                node->pointers.children[j+2] = node->pointers.children[j+1];
            }
            node->keys[i] = childPromotedKey;
            node->pointers.children[i+1] = childNew;
            node->numKeys++;
            *promotedKey = -1;
            *newChild = NULL;
            return node;
        } else {
            /* Split Internal Node (similar to splitting leaf) */
            // Allocate temporary space for keys + children
            int total = node->numKeys + 1; // total children count after insertion
            int *tempKeys = (int*) malloc(sizeof(int) * (total));
            BTreeNode **tempChildren = (BTreeNode**) malloc(sizeof(BTreeNode*) * (total + 1));
            int j, k = 0;
            // Merge existing keys and new key
            for (j = 0; j < node->numKeys; j++, k++) {
                if (k == i + 1) {
                    tempChildren[k] = childNew;
                    k++;
                }
                tempKeys[j] = node->keys[j];
                tempChildren[k] = node->pointers.children[j];
            }
            if (i + 1 >= total)
                tempChildren[i+1] = childNew;
            // For simplicity, assume total keys = order + 1 and split at index splitIdx:
            int splitIdx = total / 2;
            int promoted = tempKeys[splitIdx];
            BTreeNode *newInternal = createInternalNode(order);
            newInternal->numKeys = total - splitIdx - 1;
            for (j = 0; j < newInternal->numKeys; j++) {
                newInternal->keys[j] = tempKeys[splitIdx + 1 + j];
            }
            for (j = 0; j < newInternal->numKeys + 1; j++) {
                newInternal->pointers.children[j] = tempChildren[splitIdx + j];
            }
            node->numKeys = splitIdx;
            free(tempKeys);
            free(tempChildren);
            *promotedKey = promoted;
            *newChild = newInternal;
            return node;
        }
    }
}

/* ============================================================================
 * Index Manager Initialization (No Global Setup Required)
 * ============================================================================
 */
RC initIndexManager(void *mgmtData) {
    (void)mgmtData;
    return RC_OK;
}

/* ============================================================================
 * Shutdown Index Manager (No Cleanup Required Globally)
 * ============================================================================
 */
RC shutdownIndexManager(void) {

    // No global shutdown needed.
    return RC_OK;
}

/* ============================================================================
 * Create a New B+ Tree Index File
 * ============================================================================
 *
 * This project uses in-memory simulation, so this just creates a page file.
 * Keys are integers only (DT_INT).
 */
RC createBtree(char *idxId, DataType keyType, int n) {
    (void)*idxId;
    (void)keyType;
    (void)n;
    if (keyType != DT_INT)
        return RC_IM_KEY_NOT_FOUND;  // Only DT_INT keys supported
    RC rc = createPageFile(idxId);
    if (rc != RC_OK)
        return rc;
    // For this simplified implementation, we do not persist the in‑memory tree.
    return RC_OK;
}

/* ============================================================================
 * Open an Existing B+ Tree
 * ============================================================================
 *
 * Allocate and initialize the BTreeHandle with metadata.
 */
RC openBtree(BTreeHandle **tree, char *idxId) {
    (void)**tree;
    (void)*idxId;
    if (!tree || !idxId)
        return RC_ERROR;
    BTreeHandle *btree = (BTreeHandle*) malloc(sizeof(BTreeHandle));
    if (!btree)
        return RC_MALLOC_FAILED;
    btree->keyType = DT_INT;
    btree->idxId = strdup(idxId);
    BTreeMgmtData *mgmt = (BTreeMgmtData*) malloc(sizeof(BTreeMgmtData));
    if (!mgmt) {
        free(btree);
        return RC_MALLOC_FAILED;
    }
    // For simplicity, set the order to a fixed value (or read from file metadata)
    mgmt->order = 3;
    mgmt->root = NULL;
    mgmt->numNodes = 0;
    mgmt->numEntries = 0;
    btree->mgmtData = mgmt;
    *tree = btree;
    return RC_OK;
}

/* ============================================================================
 * Recursively Free All Nodes in the Tree
 * ============================================================================
 */
static void freeBTreeNode(BTreeNode *node, int order) {
    (void)*node;
    (void)order;
    if (!node) return;
    if (node->type == INTERNAL) {
        for (int i = 0; i <= node->numKeys; i++) {
            freeBTreeNode(node->pointers.children[i], order);
        }
        free(node->keys);
        free(node->pointers.children);
    } else {  // LEAF
        free(node->keys);
        free(node->pointers.rids);
        // Do NOT free node->next here because it is managed via parent's pointers.
    }
    free(node);

}

/* ============================================================================
 * Close the B+ Tree - Free Memory and Deallocate Tree Handle
 * ============================================================================
 */
RC closeBtree(BTreeHandle *tree) {
    (void)*tree;
    if (!tree)
        return RC_ERROR;
    BTreeMgmtData *mgmt = (BTreeMgmtData*) tree->mgmtData;
    if (mgmt && mgmt->root)
        freeBTreeNode(mgmt->root, mgmt->order);
    if (mgmt)
        free(mgmt);
    free(tree->idxId);
    free(tree);
    return RC_OK;
}

/* ============================================================================
 * Delete B+ Tree - Remove the Page File
 * ============================================================================
 */
RC deleteBtree(char *idxId) {
    (void)*idxId;
    return destroyPageFile(idxId);
}

/*
 * Retrieves the total number of nodes in the B⁺‑tree.
 */
RC getNumNodes(BTreeHandle *tree, int *result) {
    (void)*tree;
    (void)*result;
    if (!tree || !result)
        return RC_ERROR;
    BTreeMgmtData *mgmt = (BTreeMgmtData*) tree->mgmtData;
    *result = mgmt->numNodes;
    return RC_OK;
}

/*
 * Retrieves the total number of entries in the B⁺‑tree.
 */
RC getNumEntries(BTreeHandle *tree, int *result) {
    (void)*tree;
    (void)*result;
    if (!tree || !result)
        return RC_ERROR;
    BTreeMgmtData *mgmt = (BTreeMgmtData*) tree->mgmtData;
    *result = mgmt->numEntries;
    return RC_OK;
}

/*
 * Retrieves the key type of the B⁺‑tree.
 */
RC getKeyType(BTreeHandle *tree, DataType *result) {
    (void)*tree;
    (void)*result;
    if (!tree || !result)
        return RC_ERROR;
    *result = tree->keyType;
    return RC_OK;
}

/*
 * Searches for a key in the B⁺‑tree and returns its corresponding record ID.
 */
RC findKey(BTreeHandle *tree, Value *key, RID *result) {
    (void)*tree;
    (void)*key;
    (void)*result;
    if (!tree || !key || key->dt != DT_INT || !result)
        return RC_ERROR;
    BTreeMgmtData *mgmt = (BTreeMgmtData*) tree->mgmtData;
    int searchKey = key->v.intV;
    BTreeNode *leaf = findLeaf(mgmt->root, searchKey);
    if (!leaf)
        return RC_IM_KEY_NOT_FOUND;
    for (int i = 0; i < leaf->numKeys; i++) {
        if (leaf->keys[i] == searchKey) {
            *result = leaf->pointers.rids[i];
            return RC_OK;
        }
    }
    return RC_IM_KEY_NOT_FOUND;
}

/*
 * Inserts a new key and record ID pair into the B⁺‑tree.
 * Splits nodes as necessary and adjusts the root if required.
 */
RC insertKey(BTreeHandle *tree, Value *key, RID rid) {
    (void)*tree;
    (void)*key;
    (void)rid;
    if (!tree || !key || key->dt != DT_INT)
        return RC_ERROR;
    BTreeMgmtData *mgmt = (BTreeMgmtData*) tree->mgmtData;
    int intKey = key->v.intV;
    int promotedKey;
    BTreeNode *newChild = NULL;
    if (mgmt->root == NULL) {
        // Tree is empty; create a new leaf node as root.
        mgmt->root = createLeafNode(mgmt->order);
        insertIntoLeaf(mgmt->root, intKey, rid, mgmt->order);
        mgmt->numNodes++;
        mgmt->numEntries++;
        return RC_OK;
    }
    mgmt->root = insertRecursive(mgmt->root, intKey, rid, mgmt->order, &promotedKey, &newChild);
    if (newChild != NULL) {
        // Root was split; create a new root node.
        BTreeNode *newRoot = createInternalNode(mgmt->order);
        newRoot->numKeys = 1;
        newRoot->keys[0] = promotedKey;
        newRoot->pointers.children[0] = mgmt->root;
        newRoot->pointers.children[1] = newChild;
        mgmt->root = newRoot;
        mgmt->numNodes++;
    }
    mgmt->numEntries++;
    return RC_OK;
}

/*
 * Deletes a key from the B⁺‑tree.
 * (Deletion and underflow handling are not implemented in this simplified version.)
 */
RC deleteKey(BTreeHandle *tree, Value *key) {
    if (!tree || !key || key->dt != DT_INT)
        return RC_ERROR;

    BTreeMgmtData *mgmt = (BTreeMgmtData*) tree->mgmtData;
    int searchKey = key->v.intV;
    BTreeNode *leaf = findLeaf(mgmt->root, searchKey);
    if (!leaf)
        return RC_IM_KEY_NOT_FOUND;

    int i, found = 0;
    for (i = 0; i < leaf->numKeys; i++) {
        if (leaf->keys[i] == searchKey) {
            found = 1;
            break;
        }
    }
    if (!found)
        return RC_IM_KEY_NOT_FOUND;

    // Remove the key by shifting keys and RIDs left.
    for (int j = i; j < leaf->numKeys - 1; j++) {
        leaf->keys[j] = leaf->keys[j+1];
        leaf->pointers.rids[j] = leaf->pointers.rids[j+1];
    }
    leaf->numKeys--;
    mgmt->numEntries--;

    return RC_OK;
}


/* --- B⁺‑Tree Scan Functions --- */

/* Structure to hold scan state */
typedef struct BT_ScanMgmtData {
    BTreeNode *currentLeaf;
    int currentIndex;
} BT_ScanMgmtData;

/*
 * Opens a tree scan by initializing the scan state to the leftmost leaf.
 */
RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle) {
    (void)*tree;
    (void)**handle;
    if (!tree || !handle)
        return RC_ERROR;
    BT_ScanHandle *scan = (BT_ScanHandle*) malloc(sizeof(BT_ScanHandle));
    if (!scan)
        return RC_MALLOC_FAILED;
    scan->tree = tree;
    BT_ScanMgmtData *scanData = (BT_ScanMgmtData*) malloc(sizeof(BT_ScanMgmtData));
    if (!scanData) {
        free(scan);
        return RC_MALLOC_FAILED;
    }
    BTreeMgmtData *mgmt = (BTreeMgmtData*) tree->mgmtData;
    BTreeNode *node = mgmt->root;
    while (node && node->type != LEAF)
        node = node->pointers.children[0];
    scanData->currentLeaf = node;
    scanData->currentIndex = 0;
    scan->mgmtData = scanData;
    *handle = scan;
    return RC_OK;
}

/*
 * Retrieves the next entry (record ID) in the B⁺‑tree scan.
 */
RC nextEntry(BT_ScanHandle *handle, RID *result) {
    (void)*handle;
    (void)*result;
    if (!handle || !result)
        return RC_ERROR;
    BT_ScanMgmtData *scanData = (BT_ScanMgmtData*) handle->mgmtData;
    if (!scanData || !scanData->currentLeaf)
        return RC_IM_NO_MORE_ENTRIES;
    if (scanData->currentIndex >= scanData->currentLeaf->numKeys) {
        scanData->currentLeaf = scanData->currentLeaf->next;
        scanData->currentIndex = 0;
        if (!scanData->currentLeaf)
            return RC_IM_NO_MORE_ENTRIES;
    }
    *result = scanData->currentLeaf->pointers.rids[scanData->currentIndex];
    scanData->currentIndex++;
    return RC_OK;
}

/*
 * Closes a tree scan and frees associated resources.
 */
RC closeTreeScan(BT_ScanHandle *handle) {
    (void)*handle;
    if (!handle)
        return RC_ERROR;
    if (handle->mgmtData)
        free(handle->mgmtData);
    free(handle);
    return RC_OK;
}

/* --- Debug Function --- */

/*
 * Generates a string representation of the B⁺‑tree in depth‑first pre‑order.
 * Each node is printed as: (pos)[key1,key2,...]
 * Node positions are based on a depth‑first pre‑order traversal.
 * The returned string is dynamically allocated and should be freed by the caller.
 *
 * Note: This implementation assumes the existence of a VarString utility.
 */
#ifdef USE_VARSTRING  // Assuming VarString macros (MAKE_VARSTRING, APPEND_STRING, RETURN_STRING) are defined
#include "varstring.h"
#else
/* If VarString is not available, a simplified implementation could be provided. */
#endif

/* ============================================================================
 * Recursive Helper to Print Tree Nodes (Depth-First Pre-Order)
 * ============================================================================
 */
static void printTreeNode(BTreeNode *node, int *nodeCounter, char *buffer, int bufSize) {
    (void)*node;
    (void)*nodeCounter;
    (void)*buffer;
    (void)bufSize;
    if (!node) return;
    char line[256];
    snprintf(line, sizeof(line), "(%d)[", *nodeCounter);
    strcat(buffer, line);
    for (int i = 0; i < node->numKeys; i++) {
        char keyStr[32];
        snprintf(keyStr, sizeof(keyStr), "%d", node->keys[i]);
        strcat(buffer, keyStr);
        if (i < node->numKeys - 1)
            strcat(buffer, ",");
    }
    strcat(buffer, "]\n");
    (*nodeCounter)++;
    if (node->type == INTERNAL) {
        for (int i = 0; i <= node->numKeys; i++) {
            printTreeNode(node->pointers.children[i], nodeCounter, buffer, bufSize);
        }
    }
}

/* ============================================================================
 * Print the Entire Tree in Pre-Order to a Dynamically Allocated String
 * ============================================================================
 */
char *printTree(BTreeHandle *tree) {
    if (!tree) return NULL;
    BTreeMgmtData *mgmt = (BTreeMgmtData*) tree->mgmtData;
    int nodeCounter = 0;
    // Allocate a buffer for the tree string
    int bufSize = 8192;
    char *buffer = (char*) malloc(bufSize);
    if (!buffer) return NULL;
    buffer[0] = '\0';
    printTreeNode(mgmt->root, &nodeCounter, buffer, bufSize);
    return buffer;
}