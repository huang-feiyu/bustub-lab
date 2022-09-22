# Buffer Pools

> How the DBMS manages its memory and **move data** back-and-forth from disk?

[TOC]

```diff
  Query Planning
  Operator Execution
  Access Methods
+ Buffer Pool Manager <- Today
- Disk Manager <- Done

Bottom-up
```

Bifurcated Environment:

```
   [1] -\   Extract     +----+
   [2] --> Transform => |    |
   [3] -/    Load       +----+
        <=---------------
OLTP Data Silos       OLAP warehouse
```

### Database Storage

Spatial Control: Where to write pages to disk => Keep pages that are used together also close together on disk

Temporal Control: When to read/write pages => Minimize reading from disk

### Buffer Pool Manager

> DB 自己管理内存，同时跟踪事务等如何修改 page

In buffer pool, memory is organized as an array of *fixed-size* pages => **frame** (与 slot 在 Page 中的地位一样，用作 offset)

**page table** keeps track of pages that are currently in memory, maintain additional meta-data per page

* Dirty Flag: 脏位，
* Pin/Reference Counter: 正在使用该 page 的数量

---

Locks & Latches

* Lock: 高级逻辑原语，Held for transaction duration; Need be able to rollback
* Latch: 底层保护原语，Held for operation duration; Protect critical sections of internal data（OS 中的 Mutex 锁）

---

Page Table & Page Directory

* Page Directory: Database File 中，Page ids 与 Page location 的映射（持久化要求）
* Page Table: Buffer Pool frames 中，page ids 与 page 拷贝的映射

---

Allocation Policies

* Global Policies: 针对所有的 active 事务进行优化
* Local Policies: 针对一个事务、查询进行优化

---

Buffer Pool Optimizations

* Multiple Buffer Pools
  * Object Id: 给定专门 record id，维护 tuple 与特定 Buffer Pool 之间的映射
  * Hashing: hash page id 以选择哪个 Buffer Pool
* Pre-Fetching：根据查询计划提前复制 page 到内存中
* Scan Sharing：如果两个查询在做同样的事，那么一个会直接搭便车到另一个上（保存自己原有的东西）
* Buffer Pool Bypass

To completely control how the data write to the disk, we use direct I/O API.

### Replacement Policies

Goal: Correctness; Accuracy; Speed; Meta-data overhead

* LRU: 跟踪上一次访问的 timestamp；使用队列
* Clock: Approximation of LRU
  * 仅仅跟踪 page 的 reference bit，当 page 被访问，设为 1
  * Refers to PPT

To avoid sequential flooding

* LRU-K: Learn from history K references
* Localization
* Priority Hint

---

Dirty Pages: trade-off

* FAST: not dirty, then drop it
* SLOW: dirty, write back and read back

=> Background Writing: 周期性扫描 page table，写入 dirty page

### Other Memory Pools

DBMS needs memory for NOT only tuple and indexes

Other memory pools may not always backed by disk, depends on implementation

