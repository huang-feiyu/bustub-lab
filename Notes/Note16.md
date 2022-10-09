# Concurrency Control Theory

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

For data integrity & persistence, we need two valuable properties of DBMSs:

* Concurrency Control: **Correct Order**
  * Operator Execution
  * Access Methods
  * Buffer Pool Manager
* Recovery: **Persistence**
  * Buffer Pool Manager
  * Disk Manager

### Transaction

***Transaction***: the execution of a sequence of one or more operations (e.g., SQL queries) on a database to perform some **higher-level function**.

txn is the *basic unit* of change in a DBMS.

For better throughput, a better approach is to allow **concurrent execution** of independent txns.

Goal: Correct & Quick

---

In practice, a new txn starts with `BEGIN`, stops with `COMMIT` (commit **or** abort) or `ABORT`.

### Correctness Criteria

> ACID

* **Atomicity**: either all or nothing
  * Approach #1: Write-ahead **Logging**
  * Approach #2: Shadow Paging (slow)
* Consistency: it looks "correct" to me (logical correct)
  * Database Consistency: The database accurately models the real world and follows *integrity constraints* (logical correct: e.g., age always greater than 0);<br/>In other words, txns in the future can see the effects of txns committed past. (Matters more in distributed system)
  * Transaction Consistency: We cannot do this inside DBMS
* **Isolation**: as if run alone
  * concurrency control protocol: how the DBMS decides the *proper interleaving* of operations from multiple transactions
  * Pessimistic & Optimistic
  * We wants the **final state** is equivalent to serial schedule, which gives us more choices to manipulate txns
  * Conflict Serializability: use dependency graph, if there is cycle, it is un-serializable
  * View Serializability: Theoretical, requires us to explain the txn logic
* Durability: survive failures

```bash
# Isolation: schedules
All schedules > view serializable > confict serializable > serial
```

Concurrency control is **automatic**.

