# Database Recovery

[TOC]

```diff
-     Query Planning
  +-----------------------+
- | Concurrency Control   |
  -------------------------
- |   Operator Execution  |
- |   Access Methods      |
  | +-----------------------+
+ | |  Logging & Recovery | | <- Today
  | ----------------------|--
- | | Buffer Pool Manager | |
  +-|-------------------- + |
-   | Disk Manager          |
    +-----------------------+
```

When crash or failure, we need to persist our data.

Crash Recovery

1. At runtime, ensure DBMS can recover from a failure. Collect information
2. After a **failure/reboot**, ensure A (atomicity), C (consistency), D (durability). Make sure database at a correct state

ARIES: Algorithms for Recovery and Isolation Exploiting Semantics

* Write-Ahead Logging
* Repeating History During Redo
* Logging Changes During Undo

### Log Sequence Numbers

LSN, Log Sequence Numbers: Unique counter for log record

Multiple types of LSN make sure we can do stuff easily.

### Normal Commit & Abort Operations

txn invokes a sequence of reads/writes, followed by commit or abort

* txn commit
  * COMMIT record
  * log records of the commit
  * TXN-END record if succeeds
* txn abort
  * ABORT record
  * play back the txn's updates in reverse order<br/>Write CLR to the log<br/>Restore old value
  * TXN-END record if ends
  * Add prevLSN to log records, maintain a linked-list for each txn that makes it easy to walk through its records
  * CLR, Compensation Log Records: describes the actions taken to undo the actions of a previous update record (Add undoNext to log records for convenience)

### Fuzzy Checkpointing

Non-Fuzzy Checkpoints: DBMS halts everything when it takes a checkpoint to ensure a consistent snapshot => Bad

fuzzy-checkpoints: do not stall transaction, CHECKPOINT-BEGIN & CHECKPOINT-END for boundaries

* Active Transaction Table: Entry(txn_id, status, lastLSN)
* Dirty Page Table: dirty pages in buffer pool

### Recovery Algorithm

* Phase #1: Analysis
  * Read WAL to last checkpoint to identify dirty pages and active txns
* Phase #2: Redo
  * Repeat all actions starting from appropriate point in the log (even txns that will abort)
* Phase #3: Undo
  * Reverse the actions of txns that did not commit before the crash

