# Assignment 4: B+ Tree Index Implementation
### CS 525 Advanced Database Organization (Spring 2025)

Professor: Gerald Balekaki\
TAs: Mohammadreza Sediqin, Nency Patel, Rashmi Topannavar, Ramani Chinthakindi\
Students: Harlee Ramos, Jisun Yun, Baozhu Xie

Loom video: https://www.loom.com/share/061a287e4fa04a33b2eac1afc51887f5?sid=561ffcb8-537e-46aa-b5e9-20f8bde1291c
---

## Project Overview

This assignment focuses on implementing a **B+ Tree Index Manager**. The index manager is built on top of existing components:

- **Storage Manager**
- **Buffer Manager**
- (Optionally) **Record Manager**

The system supports **integer keys (`DT_INT`)** and performs efficient insertion, deletion, search, and range scan operations. Each node in the B+ Tree (leaf or internal) occupies a full page on disk. The structure follows the canonical B+ Tree rules for managing entries, redistributing values, and maintaining balance.

---

## Compilation and Execution Instructions

### 1. Navigate to the project directory:
```bash
cd assign4
```

### 2. Clean any previous builds:
```bash
make clean
```

### 3. Compile the project:
```bash
make all
```

### 4. Run test suite `test_assign4_1.c`:
```bash
make run_test1
```

---

## Implementation Highlights

- Fully modular design with separation between index logic and file/storage operations
- B+ Tree supports recursive insertion, splitting, merging, and deletion
- Optimized for integer key indexing and in-order traversal
- Compatible with buffer and storage layers for persistence simulation
- Extensible architecture for future support of string/float keys

---

## Component Overview

### 1. B+ Tree Functionalities

**Search and Retrieval**
- `findLeaf()`: Identifies the appropriate leaf node for a given key
- `findKey()`: Retrieves the record ID associated with a given key

**Insertion**
- `insertKey()`: Inserts a key and associated record ID
- `splitLeaf()`, `insertIntoParent()`: Handles recursive splits and updates to internal nodes

**Deletion**
- `deleteKey()`: Removes a key and updates the tree structure
- `mergeNodes()`, `redistributeNodes()`: Resolves underflows and maintains balance

**Tree Statistics**
- `getNumNodes()`, `getNumEntries()`: Retrieves metadata such as node and entry count
- `printTree()`: Serializes the current structure of the B+ Tree

**Scanning Interface**
- `openTreeScan()`, `nextEntry()`, `closeTreeScan()`: Provides in-order traversal of all keys in the tree

---

## Test Coverage

| Test File            | Description                                              |
|----------------------|----------------------------------------------------------|
| `test_assign4_1.c`   | Insertion, deletion, search, and full tree scan          |
| `test_expr.c`        | Validation of comparison and boolean logic for `Value`   |

---

## Directory Structure

```
assign4/
├── include/
│   ├── btree_mgr.h
│   ├── buffer_mgr.h
│   ├── buffer_mgr_stat.h
│   ├── config.h
│   ├── dberror.h
│   ├── dt.h
│   ├── expr.h
│   ├── record_mgr.h
│   ├── rm_serializer.h
│   ├── storage_mgr.h
│   ├── tables.h
│   └── test_helper.h
│
├── src/
│   ├── btree_mgr.c
│   ├── buffer_mgr.c
│   ├── buffer_mgr_stat.c
│   ├── dberror.c
│   ├── expr.c
│   ├── record_mgr.c
│   ├── rm_serializer.c
│   ├── storage_mgr.c
│   ├── tables.c
│   ├── test_assign4_1.c
│   └── test_expr.c
│
├── Makefile (platform specific)
├── README.md
└── Platform Makefiles:
    ├── macOS_Arm64_M2Pro.txt
    ├── UbuntuARM64.txt
    ├── Win_11_Arm64Intel.txt
    └── WIN_11_Arm64.txt
```

## Components Description
+ `storage_mgr.[c|h]`      | **Storage Manager Module:** Provides a low-level file management interface for page‑based storage. It handles creation, destruction, opening, closing, reading, and writing of fixed‑size pages to disk. This module abstracts the underlying file I/O and simulates persistence for B⁺‑tree nodes (each occupying one page).

+ `record_mgr.[c|h]`       | **Record Manager Module:** Manages high-level record operations on tables. It supports creating tables, defining schemas, and performing record insertions, deletions, updates, and scans. It leverages the Buffer Manager for physical I/O and can integrate the B⁺‑tree index for key‑based lookups.

+ `tables.[c|h]`           | **Table & Schema Management Module:** Defines the data structures and helper routines required to represent table metadata and schemas. It facilitates attribute definitions and schema validation, ensuring that record data is properly structured and maintained.

+ `expr.[c|h]`             | **Expression Evaluation Engine:** Implements arithmetic operations, logical/Boolean expressions, and comparison operators for various `Value` types (e.g., `DT_INT`, `DT_STRING`, `DT_FLOAT`). It is utilized during record scanning to evaluate conditions similarly to SQL WHERE clauses.

+ `rm_serializer.[c|h]`    | **Record Serializer Module:** Offers routines for serialization and deserialization of records, table metadata, and schemas. It converts in-memory data structures into string representations (and vice versa) for debugging, logging, and data exchange purposes.

+ `dberror.[c|h]`          | **Database Error Handling Utilities:** Provides a unified set of error codes, macros, and functions to standardize error reporting across all modules. It ensures consistent handling and reporting of errors (e.g., `RC_OK`, `RC_ERROR`, `RC_IM_KEY_NOT_FOUND`) throughout the project.

+ `btree_mgr.[c|h]`        | **B⁺ Tree Index Manager Module:** Implements all core B⁺‑tree operations including recursive key insertion (with node splitting and key promotion), deletion (with underflow handling), key search, tree scanning (via linked leaf nodes), and printing (using depth‑first pre‑order traversal). It simulates persistence by integrating with the Storage and Buffer Managers and adheres strictly to the assignment splitting and merging rules.

+ `buffer_mgr.[c|h]`       | **Buffer Manager Module:** Manages an in‑memory pool of disk pages to optimize I/O performance. It handles page pinning/unpinning, dirty page tracking, and replacement policies (e.g., FIFO, LRU), providing an effective caching layer for all higher‑level modules requiring page access.

+ `buffer_mgr_stat.c`      | **Buffer Manager Statistics Module:** Implements debugging and performance reporting functions for the buffer pool. It outputs details such as frame usage, dirty flags, fix counts, and the mapping of pages to buffer frames, aiding in testing and performance optimization.

+ `config.h`               | **POSIX Environment Configuration:** Defines platform‑specific macros (e.g., `_POSIX_C_SOURCE 200809L`) to enable POSIX functions such as `getline()` and `strnlen()`. It also prevents multiple inclusions of the header.

+ `dt.h`                   | **Data Type Definitions Module:** Specifies the standard data types (`DT_INT`, `DT_STRING`, `DT_FLOAT`, etc.) and boolean value types used throughout the project. This module provides a consistent interface for data type management across all components.

+ `test_assign4_1.c`       | **B⁺ Tree Functional Testing Module:** Contains comprehensive test cases verifying the correct implementation of the B⁺‑tree index manager. These tests cover insertion, deletion, search, scanning, and structural validation to ensure that the index manager meets the assignment requirements.

+ `test_expr.c`            | **Expression Evaluation Testing Module:** Provides test cases to validate the functionality of the expression evaluation engine. It tests arithmetic operations, logical conditions, and comparisons on various data types to ensure correct behavior of the `expr.[c|h]` module.

+ `test_helper.h`          | **Test Utilities & Macros:** Contains helper functions and macros (e.g., `ASSERT_TRUE`, `TEST_DONE`) that standardize and simplify unit test development across all modules, ensuring consistency in test output and error reporting.


---

## Platform-Specific Build Instructions

### macOS (ARM64 - M2/M3 Chip)

Compiler: `clang`

```bash
make
make run_test1
make clean
make deepclean
```

> Refer to: `macOS_Arm64_M2Pro.txt`

---

### Ubuntu (ARM64)

Compiler: `clang`

```bash
make
make run_test1
make clean
make deepclean
```

> Refer to: `UbuntuARM64.txt`

---

### Windows 11 (Intel / ARM64 - MinGW)

Compiler: `gcc` or `arm64-w64-mingw32-gcc`

```cmd
make
make run_test1
make clean
make deepclean
```

> Refer to: `Win_11_Arm64Intel.txt`

---

### Windows Subsystem for Linux (WSL)

Compiler: `gcc`

```bash
make
make run_test1
make clean
make deepclean
```

> Refer to: `WIN_11_Arm64.txt`

---

## Example Usage in C

### Insert & Search

```c
Value *key;
RID rid = {1, 1};
MAKE_INT_VALUE(key, 42);

insertKey(tree, key, rid);

RID result;
findKey(tree, key, &result);
```

---

### Tree Scan (In-Order Traversal)

```c
BT_ScanHandle *scan;
openTreeScan(tree, &scan);

while (nextEntry(scan, &rid) == RC_OK) {
    printf("RID = %d.%d\n", rid.page, rid.slot);
}

closeTreeScan(scan);
```

---

## Potential Future Extensions

- Non-unique key support (duplicate keys)
- Additional key types (`DT_STRING`, `DT_FLOAT`)
- Improved Buffer Manager optimizations
- Persistent Tree Storage (disk-based implementation)
- Visualization of B+ Tree structures (graph output)

---
