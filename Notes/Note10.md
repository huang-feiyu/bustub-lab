# Sorting & Aggregations

[TOC]

```diff
  Query Planning
+ Operator Execution <- Today
- Access Methods
- Buffer Pool Manager
- Disk Manager <- Done
```

Query Plan

* Operators(关系代数的运算符与 tuples) are arranged in a **tree**
* Data flows from leaves **up towards** the root
* Output of root node is the result

### External Merge Sort

> Relational model/SQL is unsorted
>
> Queries can be optimized via sorting

我们可以将 tuples 排序，更高效地处理某些行为

* External Merge Sort: 对于 **runs** 进行分治排序算法
  * run: a list of k/v pairs
    * key: domain to compute the sort order
    * value: tuple/record id
  * Phase 1: Sort
  * Phase 2: Merge
* Complexity
  * #Pass=$1+\log_{B-1} (N/B)$ (-1 用于保存输出的 buffer page)
  * I/O Cost=$2N\cdot$(\#Pass)
* Optimization
  * Double Buffering

---

Use B+Tree for Sorting

* Clustered B+Tree: traverse the left-most leaf node if B+Tree use **the** key
* Unclustered B+Tree: 连续数据没有集合在一个 page，需要大量 Random I/O

### Aggregations

> Collapse multiple tuples into a single scalar value

* Sorting Aggregation
  * filter and remove redundant domains first, then sort
  * `GROUP BY`, `DISTINCT`...
* Hashing Aggregation: easy in memory
  * 建立一个临时的 hash table 扫描 table
* External Hashing Aggregation
  * Phase 1: Partition
  * Phase 2: ReHash

