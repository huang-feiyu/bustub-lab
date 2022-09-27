# Index Concurrency Control

[TOC]

```diff
  Query Planning
  Operator Execution
+ Access Methods <- Today
- Buffer Pool Manager
- Disk Manager <- Done
```

Goal

* Logical Correctness
* **Physical Correctness**

### Lock & Latches

* Locks: 保护逻辑对象，可回滚，分离事务
* **Latches**: 保护内部数据，类似于 OS mutex，分离线程

---

Latch

* Latch Modes
  * Read Mode: 其他线程可读
  * Write Mode: 独占型
* Implementations
  * Blocking OS Mutex: 退回到 OS 执行互斥操作, 可能被调度器暂停线程, `std::mutex`
  * Test-and-Set Spin Latch: 现代 CPU 实现的原子交换, `std::atomic<T>`
  * Read-Writer Latch: 使用 spinlock 实现

### Hash Table Latching

自上向下地扫描，每次获取锁顺序都是固定的

To resize the table, take a global latch on entire table

1. Page Latches: less latches, less concurrency
2. Slot Latches: more latches, more concurrency

Trade-off: Memory vs. Concurrency

### B+Tree Concurrency Control

Problems

1. Modify content of a node at the same time
2. One traversing while another splits/merges nodes

* Latch crabbing/coupling: protocol to allow multiple threads to access/modify B+Tree at the same time
  * Get latch for parent
  * Get latch for child
  * Release latch for parent if "safe"

Bottleneck: every update operations would hold write lock of root

=> Assume: 我们并不会经常使用 split/merge 操作 => 对于 parent node，我们可以使用 read lock（乐观锁）

---

leaf node scan: The leaf node sibling latch acquisition protocol must support a "no-wait" mode.

