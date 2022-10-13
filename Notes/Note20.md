# Database Logging

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

1. At **runtime**, ensure DBMS can recover from a failure. Collect information
2. After a failure/reboot, ensure A (atomicity), C (consistency), D (durability). Make sure database at a correct state

### Failure Classification

Classify different components based on storage device

* Transaction Failures
  * Logical Errors: txn cannot complete (integrity constraint violation)
  * Internal State Errors: e.g., deadlock
* System Failures
  * Software Failure: e.g., uncaught divide-by-0 exception
  * Hardware Failures: e.g., lose power<br/>Fail-stop Assumption: no un-recoverable error on disk
* Storage Media Failure: cannot handle using DBMS
  * Non-Repairable Hardware Failure: e.g., disk fire out

### Buffer Pool Policies

> Volatile memory for faster access

1. If DBMS has told someone a txn committed, it must be wrote to disk
2. No partial change

=> Undo & Redo

* Steal Policy: Whether DBMS allows an uncommitted txn to overwrite the most recent committed value in disk
* Force Policy: Whether DBMS requires that updates made by a txn are reflected on disk *before* the txn is allowed to commit (make life easier, no need to check redo info)

No-Steal + Force: 未提交的修改无法落盘 + 事务被允许提交前造成的修改必须落盘 => Easiest approach; But write set cannot run out Mem, maybe write page multiple times => **Nobody do this**

---

| Runtime Performance | No-Steal | Steal   |
| ------------------- | -------- | ------- |
| **No-Force**        | -        | Fastest |
| **Force**           | Slowest  | -       |



| Recovery Performance | No-Steal | Steal   |
| -------------------- | -------- | ------- |
| **No-Force**         | -        | Slowest |
| **Force**            | Fastet   | -       |


### Shadow Paging

> To avoid partial update

Maintain two separate copies of the database:

* Master page: Root points to master 
* Shadow: Update at shadow copy, if all done, change root pointer

Supporting rollbacks and recovery is easy.

* Undo: Remove the shadow page
* Redo: No need to do

Drawbacks: Copy page table is expensive; Commit overhead is high; Random-IO

### Write-Ahead Log

> WAL: Write-Ahead Log

Maintain a log file separate from data files that contains the changes that txns make to database

DBMS must write to disk the log file records that correspond to changes made to a database object **before** it can flush to disk

Buffer Pool Policy: Steal + No-Force => 未提交的修改可以落盘 + 事务被允许提交前造成的修改不要求落盘

---

WAL

1. DBMS stages all log records in Mem
2. In Mem, write to WAL buffer before writing data to buffer pool
3. Log records must be written to disk **before** writing data to disk

* Tags
  * <BEGIN\> record to log for each txn
  * <COMMIT\> record when a txn finishes
* Content: <T<sub>1</sub>, A, 1, 8>
  * txn id
  * obj id
  * Undo: Before-Value
  * Redo: After-Value

---

When to write log records to disk?

* When txn commits
* Can use **group commit** to batch multiple log flushes, after a certain time, write log

When to write dirty data to disk? It depends

### Logging Schemes

* Physical Logging: `git diff`
  * Record changes make to a specific location in the database
* Logical Logging: `UPDATE`, `DELETE`, `INSERT`
  * Record the high-level operations executed by txns
* Physiological Logging: Hybrid

### Checkpoints

WAL will grow forever.

Write a <CHECKPOINT\> record to log: everything before has been persisted

Redo txn which committed before crash<br/>Undo txn which not committed

---

We have to **stall all txns** when take a checkpoint

Trade-off: how often to take a checkpoint

