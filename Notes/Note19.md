# Multi-Version Concurrency Control

[TOC]

```diff
-     Query Planning
  +-----------------------+
+ | Concurrency Control   |   <- Today
  -------------------------
- |   Operator Execution  |
- |   Access Methods      |
  | +-----------------------+
  | |      Recovery       | | <- TODO
  | ----------------------|--
- | | Buffer Pool Manager | |
  +-|-------------------- + |
-   | Disk Manager          |
    +-----------------------+
```

### MVCC

Multi-Version Concurrency Control (MVCC): A way to construct a system by maintaining multiple **physical** versions of a single **logical** object in the database.

* Writers don't block readers
* Readers don't block writers

MVCC: not only a concurrency control protocol, but also affects how a DBMS manages txns

Design Decisions

* Concurrency Control Protocol
  1. Timestamp Ordering
  2. Optimistic Concurrency Control
  3. Two-Phase Locking
* Version Storage
* Garbage Collection
* Index Management

### Version Storage

**Version Chain** per logical tuple: Internal pointer field to points next/prev version of this tuple

* Append-Only Storage
  * Each version is a new physical tuple stored in the same table
* Time-Travel Storage
  * Old versions are copied to separate table space
* **Delta Storage**: similar to `git diff`
  * The original values of the **modified attributes** are copied into a separate delta record space.

### Garbage Collection

If a version is not visible to any active txn, we want to recycle the space.

* Tuple-level
  * Find old versions by examining tuples directly
  * Background Vacuuming: Separate thread periodically scan the table and find (Optimization: Maintain a bitmap)
  * Cooperative Cleaning: Worker threads check if reclaimable while walking through the version chain
* Txn-level
  * Txns keeps track of read/write set, DBMS doesn't have to scan tuples to determine visibility

### Index Management

* Logical Pointers
  * Use a fixed id per tuple
* Physical Pointers
  * Physical Pointers (version chain head) <-{table}-> Logical Pointers

