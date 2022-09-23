# Hash Tables

[TOC]

```diff
  Query Planning
  Operator Execution
+ Access Methods <- Today
- Buffer Pool Manager
- Disk Manager <- Done
```

Goal

1. Internal Meta-data
2. Core Data Storage
3. Temporary Data Structures
4. Table Indexes

Design Decisions

* Data Organization
* Concurrency

### Hash Table Intro

Hash Table: 无序键值对 + 哈希函数

* Space: $O(n)$
* Operation:
  * Average: $O(1)$ <= **matters**
  * Worst: $O(n)$

Design Decisions

1. Hash Function
   * Perfect hash function in Theory: $\text{hash(k1)} \ne \text{hash(k2)}, \text{k1}\ne\text{k2}$
   * Speed vs. Collision rate
2. Hashing Scheme
   * Handle key collisions after hashing
   * Large table vs. Extra process

### Hash Function

We do not care cryptographic, we need **fast** and **low collision rate**

Best Hash Function in practice: [XXHash3](https://github.com/Cyan4973/xxHash)

### Static Hashing Scheme

> In practice, Linear Probing crashes everything.

1. Linear Probe Hashing
   * Single giant table of slots
   * 如果已有，向后追加
   * Refers to PPT
2. Robin Hood Hashing
   * 记录与 hash 到的位置的距离数，距离越大说明越 poor
   * 碰撞后劫富济贫 => 达到平均最优
3. Cuckoo Hashing
   * 2 Hash Tables
   * 一次插入，两次哈希，通过特定的策略选择插入其中一个（hold 被打劫的那一个，带着进行下一次选择）
   * Refers to PPT

---

Non-Unique Keys

1. Separate Linked List
2. Redundant Keys

### Dynamic Hash Scheme

> Resize themselves on demand

1. Chained Hashing
   * 为每一个 key 维护一个列表 bucket（很像今天做的 [lock](https://github.com/huang-feiyu/xv6-lab/tree/lock) 中的 page table）
2. Extendible Hashing
   * 以分离的 hash key 作为索引
3. Linear Hashing
   * Refers to Video & PPT

---

We cannot use hash table index to select from a specific range.

