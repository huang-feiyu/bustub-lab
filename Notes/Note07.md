# Tree Indexes

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
4. **Table Indexes**: replica/副本 of a subset of a table's domains (efficient access) => Trade-off on overhead of indexes and speed

### B+ Tree Overview

<u>B-Tree</u> family: B-Tree, **B+Tree**, B*Tree, B<sup>link</sup>-Tree

**B+Tree**: 自平衡、有序、多路查找的树型数据结构 => $O(\log n)$

* Node
  * Inner Node: Key|Node (Degree 指向下方的指针数量)
  * Leaf Node: Key|Value (key, value 是分离的, 使用类似于 slotted page 的存储方式)
* Clustered Indexes: 根据主键排序 table

For further info, refers to slides.

### Design Decisions

> [Modern B-Tree Techniques](https://dl.acm.org/citation.cfm?id=2185842)

* Node Size: 可以将 node 看作是 page
  * the slower storage device, the larger the node size
* Merge Threshold: 可忍受的短暂不平衡
* Variable Length Keys: 存储 key 的方式
* Non-Unique Indexes
* Intra-Node Search: page 内部的搜索 (Binary search...)

### Optimization

* Prefix Compression: 前缀压缩, trie in C++ Primer
* Suffix Truncation: inner node 仅存储可区分的最短前缀
* Bulk Insert: 事先知道 index 数据，自底向上地构造树
* Pointer Swizzling

---

B+Tree is awesome.

