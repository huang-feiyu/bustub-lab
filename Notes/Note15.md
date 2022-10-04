# Query Optimization - Part II

[TOC]

```diff
+ Query Planning <- Today
- Operator Execution
- Access Methods
- Buffer Pool Manager
- Disk Manager <- Done
```

* Rules: Rewrite the query to remove inefficient things<br/>Do <u>not</u> need to examine data
* **Cost-based Search**: Use a model to estimate the cost of executing  a plan<br/>Evaluate multiple equivalent plans for a query and pick the one with the lowest cost

### Plan Cost Estimation

* How long will a query take? (CPU, DISK, Memory, Network)
* How many tuples will be read/written?

DBMS stores internal statistics about tables, attributes, and indexes in its internal catalog.

* Selection cardinality: `SC(A,R)`
  * => selectivity: `sel`, probability
  * Assumption (实际中这些假设存在问题)
    1. Uniform Data
    2. Independent Predicates
    3. Inclusion Principle
* Histograms
* Sampling: 抽样

---

由统计方法，我们得到了 Operator 的 selectivity 以及 cost of child query plan

### Plan Enumeration

DBMS will enumerate different plans for the query and estimate their costs => **choose the best plan**

* Single relation: Pick the best access method & Predicate evaluation ordering

  * Sequential Scan
  * Binary Search
  * Index Scan

* Multiple relations

  * Core assumption: only left-deep join trees are considered (pipeline model)
  * Enumerate the orderings<br/>Left-deep tree #1, Left-deep tree #2...
  * Enumerate the plans for each operator<br/>Hash, Sort-merge, Nested Loop...
  * Enumerate the access paths for each table<br/>Index #1. Index #2, Seq Scan...

* Nested Sub-queries

  * Rewrite to de-correlate and/or flatten them

  * Decompose nested query and store result to temporary table

### Summary

* Filter early as possible
* Selectivity estimations
* Dynamic programming for join orderings
* Rewrite nested queries

=> Execution Optimization is **hard**

