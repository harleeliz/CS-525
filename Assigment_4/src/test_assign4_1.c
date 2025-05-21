/************************************************************
*     File name:                test_assign4_1.c
 *     CS 525 Advanced Database Organization (Spring 2025)
 *     Harlee Ramos, Jisun Yun, Baozhu Xie
 ************************************************************/


#include <stdlib.h>
#include "dberror.h"
#include "expr.h"
#include "btree_mgr.h"
#include "tables.h"
#include "test_helper.h"

#define ASSERT_EQUALS_RID(_l, _r, message)                      \
  do {                                                          \
    ASSERT_TRUE((_l).page == (_r).page && (_l).slot == (_r).slot, message); \
  } while (0)

// Test method declarations
static void testInsertAndFind(void);
static void testDelete(void);
static void testIndexScan(void);

// Helper methods (assumed to be implemented elsewhere)
static Value **createValues(char **stringVals, int size);
static void freeValues(Value **vals, int size);
static int *createPermutation(int size);

// Global variable for test name.
char *testName;

int main(void) {
  testName = "BTree Manager Tests";

  testInsertAndFind();
  testDelete();
  testIndexScan();

  return 0;
}

// ************************************************************
void testInsertAndFind(void) {
  RID insert[] = {
    {1, 1},
    {2, 3},
    {1, 2},
    {3, 5},
    {4, 4},
    {3, 2}
  };
  int numInserts = 6;
  Value **keys;
  char *stringKeys[] = {
    "i1",
    "i11",
    "i13",
    "i17",
    "i23",
    "i52"
  };
  testName = "test b-tree inserting and search";
  int i, testint;
  BTreeHandle *tree = NULL;

  keys = createValues(stringKeys, numInserts);

  // Initialize the index manager and B-tree.
  TEST_CHECK(initIndexManager(NULL));
  TEST_CHECK(createBtree("testidx", DT_INT, 2));
  TEST_CHECK(openBtree(&tree, "testidx"));

  // Insert keys.
  for(i = 0; i < numInserts; i++) {
    TEST_CHECK(insertKey(tree, keys[i], insert[i]));
  }

  // Check index statistics.
  TEST_CHECK(getNumNodes(tree, &testint));
  ASSERT_EQUALS_INT(testint, 2, "number of nodes in btree");
  TEST_CHECK(getNumEntries(tree, &testint));
  ASSERT_EQUALS_INT(testint, numInserts, "number of entries in btree");

  // Search for keys.
  for(i = 0; i < 1000; i++) {
    int pos = rand() % numInserts;
    RID rid;
    Value *key = keys[pos];

    TEST_CHECK(findKey(tree, key, &rid));
    ASSERT_EQUALS_RID(insert[pos], rid, "did we find the correct RID?");
  }

  // Cleanup.
  TEST_CHECK(closeBtree(tree));
  TEST_CHECK(deleteBtree("testidx"));
  TEST_CHECK(shutdownIndexManager());
  freeValues(keys, numInserts);

  TEST_DONE();
}

// ************************************************************
void testDelete(void) {
  RID insert[] = {
    {1, 1},
    {2, 3},
    {1, 2},
    {3, 5},
    {4, 4},
    {3, 2}
  };
  int numInserts = 6;
  Value **keys;
  char *stringKeys[] = {
    "i1",
    "i11",
    "i13",
    "i17",
    "i23",
    "i52"
  };
  testName = "test b-tree delete";
  int i, iter;
  BTreeHandle *tree = NULL;
  int numDeletes = 3;
  bool *deletes = (bool *) malloc(numInserts * sizeof(bool));

  keys = createValues(stringKeys, numInserts);

  // Initialize index manager.
  TEST_CHECK(initIndexManager(NULL));

  // Run multiple iterations of insert-delete-search.
  for(iter = 0; iter < 50; iter++) {
    // Mark deletions.
    for(i = 0; i < numInserts; i++)
      deletes[i] = FALSE;
    for(i = 0; i < numDeletes; i++)
      deletes[rand() % numInserts] = TRUE;

    // Create B-tree.
    TEST_CHECK(createBtree("testidx", DT_INT, 2));
    TEST_CHECK(openBtree(&tree, "testidx"));

    // Insert keys.
    for(i = 0; i < numInserts; i++)
      TEST_CHECK(insertKey(tree, keys[i], insert[i]));

    // Delete entries.
    for(i = 0; i < numInserts; i++) {
      if (deletes[i])
        TEST_CHECK(deleteKey(tree, keys[i]));
    }

    // Search for keys.
    for(i = 0; i < 1000; i++) {
      int pos = rand() % numInserts;
      RID rid;
      Value *key = keys[pos];

      if (deletes[pos]) {
        int rc = findKey(tree, key, &rid);
        ASSERT_TRUE((rc == RC_IM_KEY_NOT_FOUND), "entry was deleted, should not find it");
      } else {
        TEST_CHECK(findKey(tree, key, &rid));
        ASSERT_EQUALS_RID(insert[pos], rid, "did we find the correct RID?");
      }
    }

    // Cleanup.
    TEST_CHECK(closeBtree(tree));
    TEST_CHECK(deleteBtree("testidx"));
  }

  TEST_CHECK(shutdownIndexManager());
  freeValues(keys, numInserts);
  free(deletes);

  TEST_DONE();
}

// ************************************************************
void testIndexScan(void) {
  RID insert[] = {
    {1, 1},
    {2, 3},
    {1, 2},
    {3, 5},
    {4, 4},
    {3, 2}
  };
  int numInserts = 6;
  Value **keys;
  char *stringKeys[] = {
    "i1",
    "i11",
    "i13",
    "i17",
    "i23",
    "i52"
  };

  testName = "random insertion order and scan";
  int i, testint, iter, rc;
  BTreeHandle *tree = NULL;
  BT_ScanHandle *sc = NULL;
  RID rid;

  keys = createValues(stringKeys, numInserts);

  // Initialize index manager.
  TEST_CHECK(initIndexManager(NULL));

  for(iter = 0; iter < 50; iter++) {
    int *permute;

    // Create a permutation.
    permute = createPermutation(numInserts);

    // Create B-tree.
    TEST_CHECK(createBtree("testidx", DT_INT, 2));
    TEST_CHECK(openBtree(&tree, "testidx"));

    // Insert keys in random order.
    for(i = 0; i < numInserts; i++)
      TEST_CHECK(insertKey(tree, keys[permute[i]], insert[permute[i]]));

    // Check index statistics.
    TEST_CHECK(getNumEntries(tree, &testint));
    ASSERT_EQUALS_INT(testint, numInserts, "number of entries in btree");

    // Execute a scan – tuples should be returned in sorted order.
    openTreeScan(tree, &sc);
    i = 0;
    while((rc = nextEntry(sc, &rid)) == RC_OK) {
      RID expRid = insert[i++];
      ASSERT_EQUALS_RID(expRid, rid, "did we find the correct RID?");
    }
    ASSERT_EQUALS_INT(RC_IM_NO_MORE_ENTRIES, rc, "scan did not return RC_IM_NO_MORE_ENTRIES");
    ASSERT_EQUALS_INT(numInserts, i, "not all entries were seen");
    closeTreeScan(sc);

    // Cleanup.
    TEST_CHECK(closeBtree(tree));
    TEST_CHECK(deleteBtree("testidx"));
    free(permute);
  }

  TEST_CHECK(shutdownIndexManager());
  freeValues(keys, numInserts);

  TEST_DONE();
}

// ************************************************************
int *createPermutation(int size) {
  int *result = (int *) malloc(size * sizeof(int));
  int i;
  for(i = 0; i < size; i++)
    result[i] = i;

  for(i = 0; i < 100; i++) {
    int l, r, temp;
    l = rand() % size;
    r = rand() % size;
    temp = result[l];
    result[l] = result[r];
    result[r] = temp;
  }

  return result;
}

// ************************************************************
Value **createValues(char **stringVals, int size) {
  Value **result = (Value **) malloc(sizeof(Value *) * size);
  int i;
  for(i = 0; i < size; i++)
    result[i] = stringToValue(stringVals[i]);
  return result;
}

// ************************************************************
void freeValues(Value **vals, int size) {
  while(--size >= 0)
    free(vals[size]);
  free(vals);
}
