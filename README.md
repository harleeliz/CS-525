# CS525: Advanced Database Organization – Spring 2025

This repository contains my coursework and programming assignments for **CS525: Advanced Database Organization**, offered at the **Illinois Institute of Technology** during **Spring 2025**.  
This course provides an in-depth, hands-on look into the design and implementation of modern **Database Management Systems (DBMSs)**.

---

## Textbooks & References

Any one of the following textbooks is sufficient for this course:

- *Fundamentals of Database Systems* – Elmasri & Navathe (6th Ed.)
- *Database Management Systems* – Ramakrishnan & Gehrke (3rd Ed.)
- *Database System Concepts* – Silberschatz, Korth, & Sudarshan
- *Database Systems: The Complete Book* – Garcia-Molina, Ullman, & Widom (2nd Ed.)

---

## Repository Contents

| Folder/Path   | Description                                           |
|---------------|-------------------------------------------------------|
| `assignments/`| Source code and reports for each DBMS component assignment |

### Assignment GROUP 09
**Students**: Harlee Ramos, Jisun Yun, Baozhu Xie

---

## Key Concepts Covered

This course focuses on system-level DBMS internals and real-world implementation:

- **DBMS Architecture & Hardware Awareness**  
  Memory hierarchy, disk I/O, RAID storage, buffer/page layout

- **Storage & Buffer Management**  
  File/block organization, replacement strategies, memory-to-disk flushing

- **Record and Index Management**  
  Tuple IDs, record navigation, deletion/insertion, B+-Tree indexing

- **Query Processing & Optimization**  
  Logical plans, physical execution, join algorithms, cost estimation

- **Concurrency Control & Recovery**  
  WAL, ARIES, locking, serializability, 2PL

- **Advanced Topics (if time allows)**  
  Distributed databases, parallel execution, data warehousing, big data systems

---

## Programming Tools

- **Language**: C/C++
- **Environment**: Unix/Linux-based OS
- **Tools**: `gcc`, `make`, file I/O libraries, shell scripting

---

## Major Assignments

Each programming assignment builds upon the last to form a basic DBMS:

| Assignment | Topic           | Description                                          |
|------------|------------------|------------------------------------------------------|
| 1          | Storage Manager  | Implement disk-based block reading/writing          |
| 2          | Buffer Manager   | Implement memory block caching and eviction         |
| 3          | Record Manager   | Implement record-level insertion, deletion, navigation |
| 4          | Index Manager    | Build a disk-based B+-Tree indexing structure       |

> All assignments are written in **C/C++**, group-based, and require code demonstrations.

---

## Instructor

**Dr. Gerald Balekaki**

---

## ⚠️ Disclaimer

All code and notes in this repository are shared for academic reference only and reflect original work for **CS525** at **Illinois Institute of Technology**.
