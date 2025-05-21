
# Assignment 3:  Record Manager Project
### CS 525 Advanced Database Organization (Spring 2025)

Professor: Gerald Balekaki\
TAs: Mohammadreza Sediqin, Nency Patel, Rashmi Topannavar, Ramani Chinthakindi\
Students: Harlee Ramos, Jisun Yun, Baozhu Xie


## Introduction
This project implements a simple record management system that supports table creation, record insertion, deletion, modification, conditional scanning, and schema management. The system is designed as a C library and is built around a set of interdependent modules that manage everything from low-level storage to high-level record operations.


## Project Structure
```markdown
record_manager/
├── include/          # Header files defining interfaces, data types, and constants
│   ├── buffer_mgr.h          # Buffer Manager interface and replacement strategy definitions
│   ├── buffer_mgr_stat.h     # Functions for gathering buffer manager statistics
│   ├── dberror.h             # Error codes and error handling utilities
│   ├── expr.h                # Definitions and functions for expression evaluation (used in scans)
│   ├── record_mgr.h          # Record Manager interface for table and record operations
│   ├── rm_serializer.h       # Serialization/deserialization of records to/from disk
│   ├── storage_mgr.h         # Low-level file I/O and page management routines
│   ├── tables.h              # Table schema definitions and table management functions
│   └── test_helper.h         # Helper functions for testing
├── src/              # Source files implementing the interfaces
│   ├── buffer_mgr.c          # Implementation of the buffer manager, including caching and replacement strategies (FIFO, LRU, etc.)
│   ├── buffer_mgr_stat.c     # Implementation of functions to collect and report buffer manager statistics
│   ├── dberror.c             # Implementation of error handling functions and error message retrieval
│   ├── expr.c                # Implementation of functions to parse and evaluate scan expressions
│   ├── record_mgr.c          # Core Record Manager implementation (table creation, record operations, and scans)
│   ├── rm_serializer.c       # Code for serializing and deserializing record data for persistent storage
│   ├── storage_mgr.c         # Implementation of file management functions for reading/writing pages
│   ├── tables.c              # Functions for managing table schemas and metadata
│   ├── test_assign3_1.c      # Test file containing unit tests to validate the record manager functionality
│   ├── interactive.c         # Interactive test program for user-driven operations
│   └── test_expr.c           # Test file specifically for expression evaluation functionality
├── Makefile          # Makefile (two versions provided below for platform-specific builds)
└── README.md         # This file
```

---

## Module & Function Overview

- **Error Handling (`dberror.h` / `dberror.c`):**  
  Defines return codes (e.g., `RC_OK`, `RC_FILE_NOT_FOUND`) and provides macros (such as `THROW` and `CHECK`) and functions to print or retrieve descriptive error messages.


- **Storage Management (`storage_mgr.h` / `storage_mgr.c`):**  
  Provides low-level file operations for reading and writing pages from disk. This module is critical for managing persistent storage of tables and records.


- **Buffer Management (`buffer_mgr.h`, `buffer_mgr.c`, `buffer_mgr_stat.h`, `buffer_mgr_stat.c`):**  
  Implements the buffer manager which caches pages from disk into memory. Supports multiple page replacement strategies (FIFO, LRU, CLOCK, LFU, LRU_K) and includes additional functions to monitor performance (read/write I/O statistics).


- **Record Management (`record_mgr.h` / `record_mgr.c`):**  
  Implements the Record Manager API including:
    - **Table Operations:** Creating, opening, closing, and deleting tables.
    - **Record Operations:** Inserting, deleting, updating, and fetching records.
    - **Scanning:** Starting, iterating, and closing scans over records with optional condition evaluation.


- **Record Serialization (`rm_serializer.h` / `rm_serializer.c`):**  
  Contains functions to serialize record data to a fixed-length format for disk storage and to deserialize data back into in-memory records.


- **Table Schema Management (`tables.h` / `tables.c`):**  
  Defines the structure of a table schema and provides functions to create and manage schemas, including metadata such as attribute names, data types, and key information.


- **Expression Evaluation (`expr.h` / `expr.c`):**  
  Provides utilities for constructing and evaluating expressions, particularly for filtering records during scan operations.


- **Testing (`test_assign3_1.c`):**  
  Contains unit tests and sample usage scenarios to validate that all components (storage, buffering, and record management) are functioning as expected.

---

## Dependencies
- **C Compiler:** GCC (MinGW-w64 is recommended for Windows)
- **Make:** Optional (Makefile provided for automated builds)
- **Visual Studio Code:** Optional (for code editing and debugging)

---

## Build Instructions

### Using the Makefile (Recommended)

Two versions of the Makefile are provided—choose the one that matches your operating system.

#### Windows Makefile
For Windows systems, use the following Makefile:
```markdown
CC = gcc
CFLAGS = -g -Iinclude

SRC = \
src/record_mgr.c \
src/tables.c \
src/rm_serializer.c \
src/dberror.c \
src/buffer_mgr.c \
src/buffer_mgr_stat.c \
src/expr.c \
src/storage_mgr.c \
src/test_assign3_1.c

OBJ = $(SRC:.c=.o)

all: record_manager_test

record_manager_test: $(OBJ)
$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
$(CC) $(CFLAGS) -c $< -o $@

clean:
del /F /Q src\*.o record_manager_test 2>nul || true
```

#### Mac OS Makefile

On Mac OS, the clean command must be updated. Use the following version:

```markdown
# Mac OS Makefile

CC = gcc
CFLAGS = -g -Iinclude

SRC = \
src/record_mgr.c \
src/tables.c \
src/rm_serializer.c \
src/dberror.c \
src/buffer_mgr.c \
src/buffer_mgr_stat.c \
src/expr.c \
src/storage_mgr.c \
src/test_assign3_1.c

OBJ = $(SRC:.c=.o)

all: record_manager_test

record_manager_test: $(OBJ)
$(CC) $(CFLAGS) -o $@ $(OBJ)

%.o: %.c
$(CC) $(CFLAGS) -c $< -o $@

clean:
rm -f src/*.o record_manager_test
```

#### Building with Make

1. **Open** a terminal (or command prompt) in the project’s root folder.
2. **Compile** the project by running:
   ``
   make
   ``
   This builds the executable (`record_manager_test` on Mac OS or `record_manager_test.exe` on Windows).

3. **Clean** the build artifacts by running:
   ``
   make clean
   ``

### Manual Build Instructions

If you prefer not to use Make, compile manually with the following command:

```markdown
gcc -I include -o test_assign_1.exe src/buffer_mgr_stat.c src/buffer_mgr.c src/expr.c src/record_mgr.c src/rm_serializer.c src/storage_mgr.c src/tables.c src/dberror.c src/test_assign3_1.c
```

- The `-I include` flag specifies the directory for header files.
- The `-o test_assign_1.exe` flag names the output executable.
- If the build is successful, the executable is created in the project root folder.

---

## Execution Instructions

1. **Navigate** to the project root folder in your terminal.
2. **Run the executable:**
- **Windows:**
  ``
  .\test_assign_1.exe
  ``
- **Mac OS/Linux:**
  ``
  ./record_manager_test
  ``


#### Building with Make

1. **Open** a terminal (or command prompt) in the project’s root folder.
2. **Compile** the project by running:
    ```sh
   make all
   ```
   This builds the executable (`record_manager_test` on Mac OS or `record_manager_test.exe` on Windows).

3. **Clean** the build artifacts by running:
   ```sh
   make clean
   ```

### Manual Build Instructions

If you prefer not to use Make, compile manually with the following command:

```markdown
gcc -I include -o test_assign_1.exe src/buffer_mgr_stat.c src/buffer_mgr.c src/expr.c src/record_mgr.c src/rm_serializer.c src/storage_mgr.c src/tables.c src/dberror.c src/test_assign3_1.c
```

- The `-I include` flag specifies the directory for header files.
- The `-o test_assign_1.exe` flag names the output executable.
- If the build is successful, the executable is created in the project root folder.

---

## Execution Instructions

1. **Navigate** to the project root folder in your terminal.
2. **Run the executable:**
- **Windows:**
  ```sh
  .\test_assign_1.exe
   ```
- **Mac OS/Linux:**
  ```sh
  ./record_manager_test
   ```

View the output in the terminal.












