# Tree Indexes

[TOC]

```diff
  Query Planning
  Operator Execution
+ Access Methods <- Today
- Buffer Pool Manager
- Disk Manager <- Done
```

### Additional Index Magic

* Implicit Indexes: 如果有 primary key，DBMS 一般会自动创建一个 index 使得 primary key 唯一且完整
* Partial Indexes: 在数据子集(WHERE)上创建 index
* Covering Indexes: 查询需要的所有信息都能够在 index 中找到
* Index Include Columns: 保存特定的非索引的 domain 信息到 leaf 中
* Functional/Expression Indexes: 将 expression 产生的信息作为索引

### Tries/Radix Trees

> [2022: C++ Primer - Trie](https://15445.courses.cs.cmu.edu/fall2022/project0/)

span: 分叉的数量, degree/fan-out

Radix Tree: Omit all nodes with only a single child

### Inverted Indexes

Hash/B+Tree are not good at keyword searches.

Inverted index/Full-text search index: mapping of words to records that **contain** those

More: Phrase Searches; Proximity Searches; Wildcard Searches

