# Timestamp Ordering Concurrency Control

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

Timestamp Ordering (T/O): Determine serializability order of txns before they execute; Optimistic, somewhat pessimistic

Timestamp: TS

* monotonically increasing
* unique fixed
* Implementations: System Clock; Logical Counter; Hybrid

### Basic Timestamp Ordering

> Txns read and write objects without locks.

Every object $X$ is tagged with read/write TS:

* W-TS(X)
* R-TS(X)

Basic T/O

* Read: make sure NOT TS(T<sub>i</sub>) < W-TS(X)<br/>If is NOT, abort; Otherwise, allow read, update R-TS(X) to **max**(R-TS(X), TS(T<sub>i</sub>)), make a local copy
* Write: make sure NOT TS(T<sub>i</sub>) < R-TS(X) or TS(T<sub>i</sub>) < W-TS(X)<br/>If is NOT, abort; Otherwise, allow write, update W-TS(X), make a local copy
  * Thomas Write Rule:
    1. If TS(T<sub>i</sub>) < R-TS(X): Abort
    2. If TS(T<sub>i</sub>) < W-TS(X): Ignore the write to allow the txn to continue executing without aborting, maintain the write local copy
    3. Else: Allow write

If no Thomas Write Rule, B T/O will generates a conflict serializable schedule. (No deadlock, Possible starvation, Performance issue)

Recoverable Schedules: A schedule is recoverable if txns commit only after all txns whose changes they read, commit. (B T/O violates this)

### Protocol Optimistic Concurrency Control

> Assumption: conflicts between txns are rare and that most txns are short-lived

DBMS creates a private workspace for **each txn**: all read/write are in workspace.

OCC

* Read/Work Phase: Do txn stuff
* Validation Phase: Is there conflict or invalid, allocate timestamp. Serializable schedule & RW and WW conflicts and they are in one direction (e.g, older -> younger)
  * Backward Validation
  * Forward Validation
* Write Phase: Write to database (Only one txn can be in the Write Phase at a time)

Performance Issue: High overhead for copying data locally; More wasteful to abort a txn; Also requires latches.

### Partition-based Timestamp Ordering

Split the database up in disjoint subsets called *horizontal partitions* (shards)

Use timestamps to order txns for serial execution at each partition

### Isolation level

Phantom Problem: Insert presenting Update, there is no lock on the tuple while inserting. => Predicate Lock/Index Lock

---

Weaker levels of Isolation to improve scalability

* Isolation: Controls the extent that a txn is exposed to the actions of other concurrent txns<br/>Good for: Dirty Reads, Unrepeatable Reads, Phantom Reads

```diff
# isolation levels
+ SERILAZABLE       <- High
  REPREATABLE READS
  READ COMMITED
- READ UNCOMMITTED  <- Low
```

