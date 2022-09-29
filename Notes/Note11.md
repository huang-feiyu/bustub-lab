# Join Algorithms

[TOC]

```diff
  Query Planning
+ Operator Execution <- Today
- Access Methods
- Buffer Pool Manager
- Disk Manager <- Done
```

Join: reconstruct the *original* tuples without any information loss (Get all info from different tables)

Today: combining **2** tables at a time with **inner equijoin** algorithms (outer table => left table)

### Join Operators

Decisions

1. Output (depends on many things)
   * #1: copy values into a new output tuple
   * #2: only copy the joins keys along with record ids of the matching tuples (**late materialization**, good for column store, bad(?) in practice)
2. Cost Analysis Criteria (based on I/O cost)

No algorithms works well in all scenarios

### Nested Loop Join

> Always pick smaller table as outer table

* Table $R$: $M$ pages/blocks, $m$ tuples
* Table $S$: $N$ pages/blocks, $n$ tuples

---

* Stupid Approach: For **every** tuple in $R$, it scans $S$ once
  * Cost: $M+(m\cdot N)$

```python
foreach tuple in R:
    foreach tuple s in S:
        emit, if r and s match
```

* Block Approach
  * Cost: $M+(M\cdot N)$
  * Optimization:<br/>Use B-2 buffers for scanning the outer table.<br/>Use one buffer for the inner table, one buffer for storing output.

```python
foreach block BR ∈ R:
	foreach block BS ∈ S:
		foreach tuple r ∈ BR:
			foreach tuple s ∈ Bs:
				emit, if r and s match
```

---

Sequential scan is expensive => Use index

* Index Approach
  * Cost: $M+(m\cdot C)$

```python
foreach tuple r in R:
    foreach tuple s in Index(ri=sj):
        emit, if r and s match
```

### Sort-Merge Join

* Phase #1: Sort (can use external merge sort)
  * Sort both tables on the join key(s)
* Phase #2: Merge
  * Step through the 2 sorted tables with cursors and emit matching tuples
  * Might need to backtrack
* Cost: $2M\cdot (\log M / \log B) + 2N\cdot (\log N / \log B) + (M + N)$<br/>aka. $\text{(sort cost)} + M + N$

```python
sort R,S on join keys
cursor_R ← R_sorted, cursor_S ← S_sorted
while cursor_R and cursor_S:
	if cursor_R > cursor_S:
		increment cursor_S
	if cursor_R < cursor_S:
		increment cursor_R
	elif cursor_R and cursor_S match:
		emit
		increment cursor_S
```

When sort-merge join useful: one or both tables are already sorted on join key

### Hash Join

* Phase #1: Build
  * Scan the outer relation and populate hash table using the hash function h<sub>1</sub> on the join keys
* Phase #2: Probe
  * Scan the inner relation and use h<sub>1</sub> on each tuple to jump to a location in the hash table and find a matching tuple

```python
build hash table HT_R for R
foreach tuple s in S:
    output, if h1(s) in HT_R
```

Hash Table

* Key: The keys that query is joining on
* Value: Varies on implementations
  * #1: Full Tuple
  * #2: Tuple Identifier (record id)

---

Probe Phase Optimization(sideway information passing): Use a Bloom Filter

* Bloom Filter: Probabilistic bitmap (might false positives)
* Threads check the filter before probing the hash table

---

To avoid bpm swap out the hash table pages randomly => Grace Hash Join: Hash join when tables do not fit in memory (refers to Slide)

* Cost: $3\cdot (M + N)$

