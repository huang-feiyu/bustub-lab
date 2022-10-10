# Two-Phase locking

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

We need **a way to guarantee that all execution schedules are correct** (i.e., serializable) without knowing the entire schedule ahead of time.

Today, we use pessimistic approach: use **locks** to protect database objects.

### Lock Types

Locks: Protect high-level database objects.

| Type      | Shared | Exclusive | For    |
| --------- | ------ | --------- | ------ |
| **S**hared    | Yes    | No        | reads  |
| E**x**clusive | No     | No        | writes |

Executing with locks

1. Transactions request locks (or upgrades).
2. Lock manager grants or blocks requests.
3. Transactions release locks.
4. Lock manager updates its internal lock-table.

### Two-Phase Locking

> Two-Phase Locking: 2PL

2PL: concurrency control protocol, determines **whether a txn can access an object** in the database on the fly.

**The txn is not allowed to acquire/upgrade locks after the growing phase finishes.**

* Phase #1: Growing
  * Each txn requests the locks that it needs from lock mgr
  * The lock_mgr grants/denies lock requests
* Phase #2: Shrinking
  * The txn is allowed to only release locks that it previously acquired. It cannot acquire new locks.

There is <u>cascading aborts</u>: if a txn aborts, we must abort another running txn which depends on previous one.

2PL is somewhat strict, and still has problems:

* may still have "dirty reads"
  * Solution: Strong strict 2PL (SS2PL), release all locks at the end (no shrinking phase)<br/>Advantage: simplify cascading aborts
* may lead to deadlocks
  * Solution: Detection or Prevention

### Deadlock Detection + Prevention

> Deadlock: a cycle of txns waiting for each other

2PL may lead to deadlocks, there two ways to deal with it:

* Approach #1: Detection
  * Build a waits-for graph, check if there is a cycle
  * Handling: select a "victim" txn and kill it (rollback, restart or abort)
  * Trade-off: frequency of checking vs. how txns have to wait before deadlocks are broken => Depend on workload
* Approach #2: Prevention
  * When a txn tries to acquire a lock that is held by another txn, the DBMS kills one of them to prevent a deadlock. (same as deadlock prevention)
  * Protocol 1: Wait-Die (old waits for young, timestamp)
  * Protocol 2: Wound-Wait (young waits for old)

### Hierarchical Locking

Lock Granularity (锁粒度)

```
Database -> Table -> Tuple -> Attr
```

Intention lock: If a node is locked in an intention mode, then some txn is doing explicit locking at a lower level in the tree.

1. Intention-Shared (IS)
2. Intention-Exclusive (IX)
3. Shared+Intention-Exclusive (SIX)

